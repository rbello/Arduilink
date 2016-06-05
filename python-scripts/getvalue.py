#!/usr/bin/python
# -*- coding: utf-8 -*-

#  sudo pip install pyserial

import serial
import time
import sys

if len(sys.argv) == 1:
	print "Usage: getvalue.py <sensorId> [nodeId] [port] [baudrate]"
	sys.exit(-1)

if len(sys.argv) < 3:
	sys.argv.append("0")
if len(sys.argv) < 4:
	sys.argv.append("/dev/ttyUSB0")
if len(sys.argv) < 5:
	sys.argv.append("9600")

# Ouverture de la liaison série
ser = serial.Serial(sys.argv[3], sys.argv[4])

# Debug
#print(ser)

try:

	# Welcome message
	welcome = ser.readline()
	# Get sensor info
	#ser.write("I;0;2")
	#sys.stdout.write(ser.readline())
	# Get sensor data
	ser.write("G;" + sys.argv[2] + ";" + sys.argv[1])
	data = ser.readline().strip()
	if data.startswith("D;") == False:
		print "Error: " #+ data
		sys.exit(-3)
	else:
		toks = data.split(";") # Return an array like ['D', '<nodeId>', '<sensorId>', '<value>']
		print toks[3]
		sys.exit(0)

except  KeyboardInterrupt:
	ser.close();
	print ""
	sys.exit(-2)
