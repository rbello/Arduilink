#!/usr/bin/python
# -*- coding: utf-8 -*-

# A R D U I L I N K
# Python script

# Module "serial" is required. Install it using:
#
#  sudo pip install pyserial
#   or
#  sudo pip3 install pyserial

import serial
import getopt
import sys
import time
import socket
import time
from threading import *

# Default values
serialPort = '/dev/ttyUSB0'
serialBaudrate = '9600'
netPort = '1000'
debug = False

# Handle CLI arguments
try:
      opts, args = getopt.getopt(sys.argv[1:], 'hf:r:p:', ['help', 'file=', 'rate=', 'port=', 'debug'])
except getopt.GetoptError as err:
      print 'Error: ' + str(err)
      sys.exit(-1)

# Configuration
for opt, arg in opts:
	if opt in ('-h', '--help'):
		print 'arduilink_server.py <-option> [args...]'
		print 'Options are:'
		print ' -f /dev/ttyUSB0		Change arduino to given port'
		print ' -r 9600			Change serial baudrate'
		print ' -p 1000			Change network port to open server socket'
		sys.exit(0)
	if opt in ('-f', '--file'):
		serialPort = arg
		continue
	if opt in ('-r', '--rate'):
		serialBaudrate = arg
		continue
	if opt in ('-p', '--port'):
		netPort = arg
		continue
	if opt in ('--debug'):
		print "Debug=True"
		debug = True
		continue

# Working vars
sock = None
list = []
arduino = None
halt = Event()
lock = Lock()
watchs = []

#
################### SERVER SOCKET
#

def start_server_socket(halt, sock, port):
	sock.listen(10)
	print time.strftime("[%Y/%m/%d %H:%M:%S]", time.gmtime()), 'Socket: listening on port ' + port + ' ...'
	while (not halt.is_set()) :
		try:
			conn, addr = sock.accept()
			#print 'Socket: connected with ' + addr[0] + ':' + str(addr[1])
			list.append(conn)
			clientThread = Thread(target = start_client_socket, args = (halt,conn,addr,))
			clientThread.setDaemon(True)
			clientThread.start()
		except (SystemExit, KeyboardInterrupt):
			break
	sock.close()

#
################### CLIENT SOCKET
#

def start_client_socket(halt, conn, addr):
	file = conn.makefile()
	while (not halt.is_set()) :
		data = file.readline()
		if not data: break
		toks = data.strip().split(';')
		
		# Requête GET : on demande une valeur de capteur
		if len(toks) == 3 and toks[0] == 'GET':
			print time.strftime("[%Y/%m/%d %H:%M:%S]", time.gmtime()), 'Socket: request=GET from=' + addr[0] + ':' + str(addr[1]) + ' sensor=' + toks[2]
			lock.acquire()
			try:
				# Write the request to the arduino
				if debug == True: print time.strftime("[%Y/%m/%d %H:%M:%S]", time.gmtime()), "	Write to arduino"
				arduino.write('GET;' + toks[1] + ';' + toks[2])
				arduino.flush()
				# Read the answer
				if debug == True: print time.strftime("[%Y/%m/%d %H:%M:%S]", time.gmtime()), "	Read from arduino"
				line = ''
				while line == '': line = arduino.readline()
				# Write-back answer to client's socket
				if debug == True: print time.strftime("[%Y/%m/%d %H:%M:%S]", time.gmtime()), "	Answer to socket: ", line.strip()
				conn.sendall(line.strip() + "\n")
			finally:
				lock.release()
				if debug == True: print time.strftime("[%Y/%m/%d %H:%M:%S]", time.gmtime()), "	Finished"
			break # Operation is done, close the socket
			
		# Requête SET : on écrit une valeur sur un capteur
		if len(toks) == 6 and toks[0] == 'SET':
			print time.strftime("[%Y/%m/%d %H:%M:%S]", time.gmtime()), 'Socket: request=SET from=' + addr[0] + ':' + str(addr[1]) + ' sensor=' + toks[2] + ' attr=' + toks[3] + ' ack=' + toks[4] + ' value=' + toks[5]
			lock.acquire()
			try:
				# Write the request to the arduino
				if debug == True: print time.strftime("[%Y/%m/%d %H:%M:%S]", time.gmtime()), "	Write to arduino"
				arduino.write('SET;' + toks[1] + ';' + toks[2] + ';' + toks[3] + ';' + toks[4] + ';' + toks[5])
				arduino.flush()
				# Read the answer (if ACK is required)
				if toks[4] == '1':
					if debug == True: print time.strftime("[%Y/%m/%d %H:%M:%S]", time.gmtime()), "	Read from arduino"
					line = ''
					while line == '': line = arduino.readline()
					if debug == True: print time.strftime("[%Y/%m/%d %H:%M:%S]", time.gmtime()), "	Answer to socket:", line.strip()
					conn.sendall(line.strip() + "\n")
			finally:
				lock.release()
				if debug == True: print time.strftime("[%Y/%m/%d %H:%M:%S]", time.gmtime()), "	Finished"
			break # Operation is done, close the socket

		# Requête WATCH : on observe un sensor
		elif len(toks) == 4 and toks[0] == 'WATCH':
			print time.strftime("[%Y/%m/%d %H:%M:%S]", time.gmtime()), 'Socket: ' + ('remove ', 'add ')[toks[3] == '1'] + addr[0] + ':' + str(addr[1]) + ' in watch list for sensor ' + toks[1] + ':' + toks[2]
			key = addr[0] + ':' + str(addr[1]) + '=' + toks[1] + ':' + toks[2]
			if toks[3] == '1':
				# Activation du mode verbose du capteur
				lock.acquire()
				try:
					arduino.write('SET;' + toks[1] + ';' + toks[2] + ';VERBOSE;1')
				finally:
					lock.release()
				# Ajout du listener
				watchs.append(key)
			else:
				# Suppression du listener
				watchs.remove(key)
				# TODO Retirer le mode verbose du capteur
		else:
			print time.strftime("[%Y/%m/%d %H:%M:%S]", time.gmtime()), 'Socket: invalid request from ' + addr[0] + ':' + str(addr[1])
			break
	# Fin du socket client
	print time.strftime("[%Y/%m/%d %H:%M:%S]", time.gmtime()), 'Socket: connexion closed ' + addr[0] + ':' + str(addr[1])
	# Nettoyage des connexions actives
	list.remove(conn)
	# Nettoyage des watchers
	k = addr[0] + ':' + str(addr[1]) + '='
	for key in watchs:
		if key.startswith(k): watchs.remove(key)
	# Fermeture du socket
	conn.close()

def broadcast_data(nodeId, sensorId, value):
	i = 0
	for sock in list:
		addr = sock.getpeername()
		key1 = addr[0] + ':' + str(addr[1]) + '=' + nodeId + ':' + sensorId
		key2 = addr[0] + ':' + str(addr[1]) + '=ALL:ALL'
		if (key1 in watchs) or (key2 in watchs):
			try:
				++i
				sock.send("DATA;{0};{1};{2}\n".format(nodeId, sensorId, value))
			except:
				continue
	print time.strftime("[%Y/%m/%d %H:%M:%S]", time.gmtime()), "Serial: received data -> node=" + str(nodeId) + " sensor=" + str(sensorId) + " value=" + str(value) + " (" + str(i) + " listener(s))"
#	
################### ARDUINO SERIAL
#

def start_serial(halt, port, baudrate):
	try:
		print time.strftime("[%Y/%m/%d %H:%M:%S]", time.gmtime()), 'Serial: connected on ' + serialPort + ' (' + serialBaudrate + ')'
		
		# Welcome sequence
		ready = False
		for i in range(1, 10):
			welcome = arduino.readline()
			if welcome.startswith('100;') == True:
				print time.strftime("[%Y/%m/%d %H:%M:%S]", time.gmtime()), 'Serial: device is ready'
				ready = True
				break
		if ready == False:
			print time.strftime("[%Y/%m/%d %H:%M:%S]", time.gmtime()), 'Serial: Invalid welcome sequence'
			arduino.close()
			sys.exit(-4)
			
		# Get sensors configuration
		arduino.write('PRESENT')

		# Read next lines
		while (not halt.is_set()):
			time.sleep(.01)
			lock.acquire()
			data = arduino.readline().strip()
			if data == '':
				lock.release()
				continue;
			if data.startswith('300;') == True:
				toks = data.split(';')
				print time.strftime("[%Y/%m/%d %H:%M:%S]", time.gmtime()), 'Serial: new sensor', toks
			elif data.startswith('200;') == True:
				toks = data.split(';')
				try:
					broadcast_data(toks[1], toks[2], toks[3])
				except Exception as e:
					print time.strftime("[%Y/%m/%d %H:%M:%S]", time.gmtime()), "Socket: error on broadcast,", e
			else:
				print time.strftime("[%Y/%m/%d %H:%M:%S]", time.gmtime()), "Serial: received info ->", data
			lock.release()
	except BaseException as error:
		print time.strftime("[%Y/%m/%d %H:%M:%S]", time.gmtime()), 'Serial: error ', sys.exc_info()[0], str(error)
		stop()
		sys.exit(-3)

#	
################### MAIN
#

def stop():
	print time.strftime("[%Y/%m/%d %H:%M:%S]", time.gmtime()), ' Server halted '
	lock.release()
	halt.set()
	for s in list: s.close()
	if sock is not None: sock.close()
	if arduino is not None: arduino.close()

try:	
	
	# Start server socket
	try:
		sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		sock.bind(('', int(netPort)))
	except socket.error as msg:
		print time.strftime("[%Y/%m/%d %H:%M:%S]", time.gmtime()), 'Socket: bind failed -> ' + msg[1] + ' (port: ' + netPort + ')'
		sys.exit(-5)
		
	# Start serial 
	try:
		#arduino = serial.Serial(serialPort, baudrate=serialBaudrate, bytesize=serial.EIGHTBITS, parity=serial.PARITY_NONE, stopbits=serial.STOPBITS_ONE, timeout=1, xonxoff=0, rtscts=0)
		arduino = serial.Serial(serialPort, serialBaudrate, timeout = 1)
		arduino.setDTR(False)
		time.sleep(.1)
		arduino.flushInput()
		arduino.setDTR(True)
	except Exception as e:
		print time.strftime("[%Y/%m/%d %H:%M:%S]", time.gmtime()), 'Serial: bind failed ->', e
		sys.exit(-6)

	# Create threads
	threadServer = Thread(target = start_server_socket, args = (halt, sock, netPort, ))
	threadServer.setDaemon(True)
	threadSerial = Thread(target = start_serial, args = (halt, serialPort, int(serialBaudrate), ))
	threadSerial.setDaemon(True)
	
	# Start threads
	print "Server running:", time.strftime("%c")
	threadServer.start()
	threadSerial.start()
	
	# Join
	while (not halt.is_set()): time.sleep(1)

except KeyboardInterrupt:
	stop()
	sys.exit(-2)
