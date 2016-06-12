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
import os

# Default values
netPort = '1000'
getTarget = None
watch = False
runScript = None

# Handle CLI arguments
try:
      opts, args = getopt.getopt(sys.argv[1:], 'hp:g:w:r:', ['help', 'port=', 'get=', 'watch=', 'run='])
except getopt.GetoptError as err:
      print 'Error: ' + str(err)
      sys.exit(-1)

# Configuration
for opt, arg in opts:
	if opt in ('-h', '--help'):
		print 'arduilink_client.py [-p port] [-g sensor] [-w sensor] [-r command]'
		print 'Options are:'
		print ' --port or -p <port>			Change server socket port to connect.'
		print ' --get or -g [node-id:]<sensor-id>	Get the current value of a given sensor.'
		print ' --watch or -w [node-id:]<sensor-id>	Watch for data send from a given sensor.'
		print ' --run or -r <command>			With watch mode, run the given command';
		print '					each time a data was received.'
		sys.exit(0)
	if opt in ('-p', '--port'):
		netPort = arg
		continue
	if opt in ('-g', '--get'):
		getTarget = arg
		continue
	if opt in ('-w', '--watch'):
		getTarget = arg
		watch = True
		continue
	if opt in ('-r', '--run'):
		runScript = arg
		continue

if getTarget is None:
	print 'Error: use -g or -w'
	sys.exit(-3)

sock = None

# On détermine le capteur cible
if ':' in getTarget:
	nodeId, sensorId = getTarget.split(':', 2)
else:
	nodeId = '0'
	sensorId = getTarget

try:
	# Connexion au serveur
	sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	sock.connect(('127.0.0.1', int(netPort)))
	
	if watch == True:
		# Watching
		sock.sendall('WATCH;' + nodeId + ';' + sensorId + ";1\n")
		file = sock.makefile()
		while 1:
			data = file.readline().strip()
			if runScript is not None:
				try:
					os.system(runScript + ' "' + data + '"')
				except Exception as e:
					print "Error: unable to run script,", e
					sys.exit(-5)
			else:
				print data
	else:
		# Get one value
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
