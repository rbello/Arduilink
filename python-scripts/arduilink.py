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

# Default values
serialPort = '/dev/ttyUSB0'
serialBaudrate = '9600'
netPort = '1000'
action = ''
getTarget = ''

# Handle CLI arguments
try:
      opts, args = getopt.getopt(sys.argv[1:], 'hf:r:p:g:', ['help', 'file=', 'rate=', 'port=', 'get='])
except getopt.GetoptError as err:
      print 'Error: ' + str(err)
      sys.exit(-1)

# Configuration
for opt, arg in opts:
	if opt in ('-h', '--help'):
		print 'arduilink.py <-option> [args...]'
		print 'Options are:'
		print ' -f /dev/ttyUSB0		Change arduino to given port'
		print ' -r 9600			Change serial baudrate'
		print ' -p 1000			Change network port to open server socket'
		print ' -g 0 1				Get value of sensor 1 of node 0'
		print ' -present			Display all sensors'
	        sys.exit()
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
	if opt in ('-g', '--get'):
		getTarget = arg
		action = 'get'
		continue

# Opening serial stream
if action == 'server' :
	print 'Connecting ' + serialPort + ' at ' + serialBaudrate

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

	if action == 'server':

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

		# Enable pooling
		arduino.write("set 0 1 verbose on")
		arduino.readline()
		arduino.write("set 0 2 verbose on")
		arduino.readline()

		# Read next lines
		while 1:
			data = arduino.readline().strip()
			if data.startswith("200 ") == True:
				toks = data.split(" ")
				print "Received data"
				print toks
			else:
				print "Received: " + data


	elif action == 'get':
		if ':' in getTarget:
			nodeId, sensorId = getTarget.split(':')
		else:
			nodeId = '0'
			sensorId = getTarget
		arduino.write('get ' + nodeId + ' ' + sensorId)
		arduino.flushOutput()
		#arduino.readline()
		r = arduino.readline().strip()
		if r.startswith('200 ') == True:
			toks = r.split(' ')
			print toks[4]
			arduino.close()
			sys.exit(0)
		else:
			print 'Error: unable to get data (' + r + ')'
			arduino.close()
			sys.exit(-5)

	else:
		arduino.close()
		print 'Error: nothing to do'
		sys.exit(-3)

except KeyboardInterrupt:
	arduino.close()
	print ""
	sys.exit(-4)
