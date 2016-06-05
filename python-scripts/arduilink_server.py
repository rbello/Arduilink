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

#	
################### SERVER SOCKET
#

def start_server_socket(halt, port):
	try:
		sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		sock.bind(('', port))
		sock.listen(10)
		print 'Socket: listening on port ' + str(port) + ' ...'
	except socket.error as msg:
		print 'Socket: bind failed -> ' + msg[1] + ' (' + str(msg[0]) + ')'
		sys.exit(-3)
	while (not halt.is_set()) :
		try:
			conn, addr = sock.accept()
			print 'Socket: connected with ' + addr[0] + ':' + str(addr[1])
			list.append(conn)
			clientThread = Thread(target = start_client_socket, args = (halt,conn,addr,))
			clientThread.setDaemon(True)
			clientThread.start()
		except (SystemExit, KeyboardInterrupt):
			sock.close()
			break

#	
################### CLIENT SOCKET
#

def start_client_socket(halt, conn, addr):
	while (not halt.is_set()) :
		data = conn.recv(1024)
		if not data: break
	print 'Socket: disconnected with ' + addr[0] + ':' + str(addr[1])
	list.remove(conn)
	conn.close()

def broadcast_data(nodeId, sensorId, value):
	for s in list:
		s.send("DATA {0} {1} {2}".format(nodeId, sensorId, value))

#	
################### ARDUINO SERIAL
#

def start_serial(halt, port, baudrate):
	try:
		print 'Serial: connecting ' + serialPort + ' at ' + serialBaudrate
		#arduino = serial.Serial(serialPort, baudrate=serialBaudrate, bytesize=serial.EIGHTBITS, parity=serial.PARITY_NONE, stopbits=serial.STOPBITS_ONE, timeout=1, xonxoff=0, rtscts=0)
		arduino = serial.Serial(serialPort, serialBaudrate)
		arduino.setDTR(False)
		time.sleep(.1)
		arduino.flushInput()
		arduino.setDTR(True)
		#print(arduino)
		# Welcome message
		welcome = arduino.readline()
		if welcome.startswith("100 ") == False:
			print 'Serial: Invalid welcome message'
			print welcome
			arduino.close()
			sys.exit(-2)
		print 'Serial: device is ready'
		# Get sensors configuration
		arduino.write("present")
		while 1:
			data = arduino.readline().strip()
			if data.startswith("300 ") == True:
				toks = data.split(" ")
				print 'Serial: new sensor'
				print toks
			else:
				break
		# Enable pooling
		arduino.write("set 0 1 verbose on")
		arduino.readline()
		arduino.write("set 0 2 verbose on")
		arduino.readline()
		# Read next lines
		while (not halt.is_set()) :
			data = arduino.readline().strip()
			if data.startswith("200 ") == True:
				toks = data.split(" ")
				broadcast_data(toks[2], toks[3], toks[4])
			else:
				print "Serial: received info -> " + data
	except BaseException as error:
		print 'Serial: exception ', sys.exc_info()[0]
		print str(error)
		stop()
		sys.exit(-2)

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
	threadServer = Thread(target = start_server_socket, args = (halt, int(netPort), ))
	threadServer.setDaemon(True)
	threadServer.start()
	# Start serial connection
	threadSerial = Thread(target = start_serial, args = (halt, serialPort, int(serialBaudrate), ))
	threadSerial.setDaemon(True)
	threadSerial.start()
	# Join on server socket lifetime
	while (threadServer.isAlive()): time.sleep(1)
	sys.exit(0)

except KeyboardInterrupt:
	stop()
	sys.exit(-1)
