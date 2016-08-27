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
import time

# Default values
netPort = '1000'
target = None
mode = None
runScript = None
value = None
debug = False

# Handle CLI arguments
try:
      opts, args = getopt.getopt(sys.argv[1:], 'hp:m:t:r:v:', ['help', 'port=', 'mode=', 'target=', 'run=', 'value=', 'debug'])
except getopt.GetoptError as err:
      print 'Error: ' + str(err)
      sys.exit(-1)

# Configuration
for opt, arg in opts:
	if opt in ('-h', '--help'):
		print 'arduilink_client.py [--debug] <-m mode> [-t targetSensor] [-p port] [-r command]'
		print 'Options are:'
		print ' --mode or -m				Modes: get, set, watch.'
		print ' --port or -p <port>		Change server socket port to connect.'
		print ' --target or -t [node-id:]<sensor-id>	Target sensor.'
		print ' --value or -v			Get the current value of a given sensor.'
		print ' --run or -r <command>			With watch mode, run the given command';
		print '					each time a data was received.'
		print ' --debug				Enable debug mode.'
		#print ' --cmd <command>				Send a server command, like STOP.'
		sys.exit(0)
	if opt in ('-p', '--port'):
		netPort = arg
		continue
	if opt in ('-m', '--mode'):
		mode = arg
		continue
	if opt in ('-t', '--target'):
		target = arg
		continue
	if opt in ('-r', '--run'):
		runScript = arg
		continue
	if opt in ('-v', '--value'):
		value = arg
		continue
	if opt in ('--debug'):
		debug = True
		continue

sock = None

# On détermine le capteur cible
if target == 'ALL': # Tous
	nodeId = sensorId = 'ALL'
elif ':' in target: # Capteur du noeud donné
	nodeId, sensorId = target.split(':', 2)
else: # Capteur du noeud par défaut (0)
	nodeId = '0'
	sensorId = target
if debug == True: print time.strftime("[%Y/%m/%d %H:%M:%S]", time.gmtime()), "Target: node=", nodeId, "sensorId=", sensorId

try:
	# Connexion au serveur
	if debug == True: print time.strftime("[%Y/%m/%d %H:%M:%S]", time.gmtime()), "Create socket"
	sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	if debug == True: print time.strftime("[%Y/%m/%d %H:%M:%S]", time.gmtime()), "Connect to 127.0.0.1:" + netPort
	sock.connect(('127.0.0.1', int(netPort)))
	if debug == True: print time.strftime("[%Y/%m/%d %H:%M:%S]", time.gmtime()), "Connected"
	
	# Set value mode
	if mode == 'set':
		if debug == True: print time.strftime("[%Y/%m/%d %H:%M:%S]", time.gmtime()), "Send SET-VALUE with value", value
		sock.sendall('SET;' + nodeId + ';' + sensorId + ";VAL;0;" + value + "\n") # TODO ACK
		if debug == True: print time.strftime("[%Y/%m/%d %H:%M:%S]", time.gmtime()), "Open file"
		file = sock.makefile()
		if debug == True: print time.strftime("[%Y/%m/%d %H:%M:%S]", time.gmtime()), "Read next line"
		data = file.readline()
		if debug == True: print time.strftime("[%Y/%m/%d %H:%M:%S]", time.gmtime()), "Handle data:", data
		if debug == True: print time.strftime("[%Y/%m/%d %H:%M:%S]", time.gmtime()), "Read next line"
		data = file.readline()
		if debug == True: print time.strftime("[%Y/%m/%d %H:%M:%S]", time.gmtime()), "Handle data:", data
		if debug == True: print time.strftime("[%Y/%m/%d %H:%M:%S]", time.gmtime()), "Close socket"
		sock.close()
		
	# Watch value mode
	elif mode == 'watch':
		if debug == True: print time.strftime("[%Y/%m/%d %H:%M:%S]", time.gmtime()), "Send WATCH message"
		sock.sendall('WATCH;' + nodeId + ';' + sensorId + ";1\n")
		if debug == True: print time.strftime("[%Y/%m/%d %H:%M:%S]", time.gmtime()), "Open file"
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
	
	# Get value mode
	elif mode == 'get':
		if debug == True: print time.strftime("[%Y/%m/%d %H:%M:%S]", time.gmtime()), "Send GET message"
		sock.sendall('GET;' + nodeId + ';' + sensorId + "\n")
		if debug == True: print time.strftime("[%Y/%m/%d %H:%M:%S]", time.gmtime()), "Open file"
		file = sock.makefile()
		if debug == True: print time.strftime("[%Y/%m/%d %H:%M:%S]", time.gmtime()), "Read next line"
		data = file.readline()
		if debug == True: print time.strftime("[%Y/%m/%d %H:%M:%S]", time.gmtime()), "Close socket"
		sock.close()
		if debug == True: print time.strftime("[%Y/%m/%d %H:%M:%S]", time.gmtime()), "Handle data:", data
		if data.startswith('200;') == True:
			toks = data.strip().split(';')
			# TODO vérifier nombre de tokens
			print toks[3]
			#print toks
			sys.exit(0)
		else:
			print 'Error:', data.strip()
			sys.exit(-4)
	else:
		print 'Error: no valid mode', mode
		sys.exit(-3)
	
except socket.error as e:
	print 'Socket error:', str(e)
	
except KeyboardInterrupt:
	print ' Interrupted '
	sys.exit(-2)

finally:
	if debug == True: print time.strftime("[%Y/%m/%d %H:%M:%S]", time.gmtime()), "End"
	if sock is not None: sock.close()
