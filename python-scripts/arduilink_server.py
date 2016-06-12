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
from threading import *

# Default values
serialPort = '/dev/ttyUSB0'
serialBaudrate = '9600'
netPort = '1000'

# Handle CLI arguments
try:
      opts, args = getopt.getopt(sys.argv[1:], 'hf:r:p:', ['help', 'file=', 'rate=', 'port='])
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
	print 'Socket: listening on port ' + port + ' ...'
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
			print 'Socket: request GET from ' + addr[0] + ':' + str(addr[1]) + ' sensor ' + toks[2]
			lock.acquire()
			try:
				arduino.write('GET;' + toks[1] + ';' + toks[2])
				line = arduino.readline()
				conn.sendall(line.strip() + "\n")
			finally:
				lock.release()
			break
		elif len(toks) == 4 and toks[0] == 'WATCH':
			print 'Socket: ' + ('remove ', 'add ')[toks[3] == '1'] + addr[0] + ':' + str(addr[1]) + ' in watch list for sensor ' + toks[2]
			key = addr[0] + ':' + str(addr[1]) + '=' + toks[1] + ':' + toks[2]
			if toks[3] == '1':
				# Activation du mode verbose du capteur
				#lock.acquire()
				#try:
				#	arduino.write('SET;' + toks[1] + ';' + toks[2] + ';VERBOSE;1')
				#finally:
				#	lock.release()
				# Ajout du listener
				watchs.append(key)
			else:
				# Suppression du listener
				watchs.remove(key)
				# TODO Retirer le mode verbose du capteur
		else:
			print 'Socket: invalid request from ' + addr[0] + ':' + str(addr[1])
			break
	# Fin du socket client
	print 'Socket: connexion closed ' + addr[0] + ':' + str(addr[1])
	# Nettoyage des connexions actives
	list.remove(conn)
	# Nettoyage des watchers
	k = addr[0] + ':' + str(addr[1]) + '='
	for key in watchs:
		if key.startswith(k): watchs.remove(key)
	# Fermeture du socket
	conn.close()

def broadcast_data(nodeId, sensorId, value):
	for socket in list:
		
		
		
		try:
			socket.send("DATA;{0};{1};{2}\n".format(nodeId, sensorId, value))
		except:
			continue
#	
################### ARDUINO SERIAL
#

def start_serial(halt, port, baudrate):
	try:
		print 'Serial: connected on ' + serialPort + ' (' + serialBaudrate + ')'
		
		# Welcome sequence
		ready = False
		for i in range(1, 10):
			welcome = arduino.readline()
			if welcome.startswith('100;') == True:
				print 'Serial: device is ready'
				ready = True
				break
		if ready == False:
			print 'Serial: Invalid welcome sequence'
			arduino.close()
			sys.exit(-4)
			
		# Get sensors configuration
		arduino.write('PRESENT')
		while 1:
			data = arduino.readline().strip()
			if data.startswith('300;') == True:
				toks = data.split(';')
				print 'Serial: new sensor'
				print toks
			else:
				break
		# Read next lines
		while (not halt.is_set()):
			time.sleep(.01)
			data = arduino.readline().strip()
			if data.startswith('200;') == True:
				toks = data.split(';')
				broadcast_data(toks[1], toks[2], toks[3])
			else:
				print "Serial: received info -> " + data
	except BaseException as error:
		print 'Serial: error ', sys.exc_info()[0], str(error)
		stop()
		sys.exit(-3)

#	
################### MAIN
#

def stop():
	print ' Server halted '
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
		print 'Socket: bind failed -> ' + msg[1] + ' (port: ' + netPort + ')'
		sys.exit(-5)
		
	# Start serial 
	try:
		#arduino = serial.Serial(serialPort, baudrate=serialBaudrate, bytesize=serial.EIGHTBITS, parity=serial.PARITY_NONE, stopbits=serial.STOPBITS_ONE, timeout=1, xonxoff=0, rtscts=0)
		arduino = serial.Serial(serialPort, serialBaudrate)
		arduino.setDTR(False)
		time.sleep(.1)
		arduino.flushInput()
		arduino.setDTR(True)
	except:
		print 'Serial: bind failed -> '
		sys.exit(-6)

	# Create threads
	threadServer = Thread(target = start_server_socket, args = (halt, sock, netPort, ))
	threadServer.setDaemon(True)
	threadSerial = Thread(target = start_serial, args = (halt, serialPort, int(serialBaudrate), ))
	threadSerial.setDaemon(True)
	
	# Start threads
	threadServer.start()
	threadSerial.start()
	
	# Join
	while (not halt.is_set()): time.sleep(1)

except KeyboardInterrupt:
	stop()
	sys.exit(-2)
