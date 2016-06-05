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
getTarget = ''

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
		print ' -g 1			Change network port to open server socket'
		sys.exit(0)
	if opt in ('-p', '--port'):
		netPort = arg
		continue
	if opt in ('-g', '--get'):
		getTarget = arg
		continue

sock = None

def linesplit(socket):
    buffer = socket.recv(256)
    buffering = True
    while buffering:
        if "\n" in buffer:
            (line, buffer) = buffer.split("\n", 1)
            yield line + "\n"
        else:
            more = socket.recv(256)
            if not more:
                buffering = False
            else:
                buffer += more
    if buffer:
        yield buffer

try:
	sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	sock.connect(('127.0.0.1', int(netPort)))
	sock.sendall("GET 0 2\r\n")
	file = sock.makefile()
	data = file.readline()
	toks = data.strip().split(' ')
	sock.close()
	print toks[4]
	sys.exit(0)
	
except socket.error as e:
	print 'Socket error:', str(e)
	
except KeyboardInterrupt:
	print ' Interrupted '
	sys.exit(-2)

finally:
	if sock is not None: sock.close()
