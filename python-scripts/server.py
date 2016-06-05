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
from threading import Thread

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
		action = 'server'
		continue

# Working vars
ss = None
list = []

# Start the server socket
def start_server_socket(port):
	try:
		ss = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		ss.bind(('', port))
		ss.listen(10)
		print 'Listening on port ' + str(port) + ' ...'
	except socket.error as msg:
		print 'Bind failed: ' + msg[1] + ' (' + str(msg[0]) + ')'
		sys.exit(-3)
	while 1:
		try:
			conn, addr = ss.accept()
			print 'Connected with ' + addr[0] + ':' + str(addr[1])
			list.append(conn)
			clientThread = Thread(target = start_client_socket, args = (conn,addr,))
			clientThread.setDaemon(True)
			clientThread.start()
		except (SystemExit, KeyboardInterrupt):
			ss.close()
			break

def start_client_socket(conn, addr):
	conn.send('Welcome to the server. Type something and hit enter\n')
	while 1:
		data = conn.recv(1024)
		if not data: break
	print 'Client disconnected: ' + addr[0] + ':' + str(addr[1])
	list.remove(conn)
	conn.close()

try:		
	threadServer = Thread(target = start_server_socket, args = (int(netPort), ))
	threadServer.setDaemon(True)
	threadServer.start()
	while (threadServer.isAlive()): time.sleep(1)
	print 'End of script'
	sys.exit(0)

except KeyboardInterrupt:
	print ' Server halted '
	if ss is not None: ss.close()
	sys.exit(-1)





# Opening serial stream
print 'Serial: connecting ' + serialPort + ' at ' + serialBaudrate
#arduino = serial.Serial(serialPort, baudrate=serialBaudrate, bytesize=serial.EIGHTBITS, parity=serial.PARITY_NONE, stopbits=serial.STOPBITS_ONE, timeout=1, xonxoff=0, rtscts=0)
arduino = serial.Serial(serialPort, serialBaudrate)
arduino.setDTR(False)
time.sleep(.1)
arduino.flushInput()
arduino.setDTR(True)
#print(arduino)



try:
	# Welcome message
	welcome = arduino.readline()
	if welcome.startswith("100 ") == False:
		print "Invalid welcome message"
		print welcome
		arduino.close()
		sys.exit(-2)
	if action == 'server':
		print "Device is ready !"

	# Get sensors configuration
	arduino.write("present")
	while 1:
		data = arduino.readline().strip()
		if data.startswith("300 ") == True:
			toks = data.split(" ")
			print "New sensor : "
			print toks
		else:
			break
	
	# Start server
	
	
	# Enable pooling
	#arduino.write("set 0 1 verbose on")
	#arduino.readline()
	#arduino.write("set 0 2 verbose on")
	#arduino.readline()

	
	
	
	# Read next lines
	while 1:
	
		
	
	
		data = arduino.readline().strip()
		if data.startswith("200 ") == True:
			toks = data.split(" ")
			print "Received data"
			print toks
		else:
			print "Received: " + data

except KeyboardInterrupt:
	arduino.close()
	print ""
	sys.exit(-4)
