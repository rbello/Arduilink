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

# Default values
netPort = '1000'
getTarget = None

# Handle CLI arguments
try:
      opts, args = getopt.getopt(sys.argv[1:], 'hp:g:', ['help', 'port=', 'get='])
except getopt.GetoptError as err:
      print 'Error: ' + str(err)
      sys.exit(-1)

# Configuration
for opt, arg in opts:
	if opt in ('-h', '--help'):
		print 'arduilink_client.py <-option> [args...]'
		print 'Options are:'
		print ' -p 1000			Change network port to open server socket'
		print ' -g 0:1			Change network port to open server socket'
		print ' -g 1			Change network port to open server socket'
		sys.exit(0)
	if opt in ('-p', '--port'):
		netPort = arg
		continue
	if opt in ('-g', '--get'):
		getTarget = arg
		continue

if getTarget is None:
	print 'Error: use -g or --get'
	sys.exit(-3)

sock = None

try:
	sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	sock.connect(('127.0.0.1', int(netPort)))
	if ':' in getTarget:
		nodeId, sensorId = getTarget.split(':', 2)
	else:
		nodeId = '0'
		sensorId = getTarget
	sock.sendall('GET;' + nodeId + ';' + sensorId + "\n")
	file = sock.makefile()
	data = file.readline()
	sock.close()
	if data.startswith('200;') == True:
		toks = data.strip().split(';')
		# TODO vérifier nombre de tokens
		print toks[3]
		sys.exit(0)
	else:
		print 'Error:', data.strip()
		sys.exit(-4)
	
except socket.error as e:
	print 'Socket error:', str(e)
	
except KeyboardInterrupt:
	print ' Interrupted '
	sys.exit(-2)

finally:
	if sock is not None: sock.close()
