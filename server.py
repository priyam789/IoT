import argparse
from time import time

import netifaces as ni
import socket
import sys

TIMEOUT = 0.1
AUTHENTICATION_INTERVAL = 10
# port = 8000
port = 8888
registeredDevices = dict()

def getServerIp(connection):
	try:
		ip = ni.ifaddresses(connection)[2][0]['addr']
		bcast = ni.ifaddresses(connection)[2][0]['broadcast']
		return ip,bcast
	except:
		print "Can't get IP for this interface. Make sure with ifconfig"
		sys.exit(0)

def validateRegistration(data):
	if(data == 'sensor' or data == 'actuator'):
		return True
	return False

def askAuthentication(comm_socket, bcast):
	global registeredDevices
	registeredDevices = {}

	print 'Asking for authentication'
	dest = (bcast, port)
	command = 'trash'
	comm_socket.sendto(command, dest)
	command = 'authenticate'
	comm_socket.sendto(command, dest)
	t0 = time()
	comm_socket.settimeout(TIMEOUT)
	while(time() - t0 < 10):
		try:
			data, address = comm_socket.recvfrom(4096)
			# print data, address
			if(validateRegistration(data)):
				comm_socket.sendto('registered', address)
				registeredDevices[address[0]] = data 		#communication standard may be implemented for messages
			else:
				comm_socket.sendto('invalid authentication', address)
		except:
			pass

	comm_socket.settimeout(None)

def decisionBox(comm_socket, address, data):
	ip = address[0]
	print 'received data %s from %s' %(data, address)
	if ip not in registeredDevices:
		comm_socket.sendto('you are not registered', address)
	else:
		comm_socket.sendto('ack', address)
		data = data.strip()
		for device in registeredDevices:
			if(registeredDevices[device] == 'actuator'):
				dest = (device, port)
				comm_socket.sendto(data, dest)



parser = argparse.ArgumentParser()
parser.add_argument('--connection', choices=['wlan0', 'eth0'], default='wlan0', help="""Describes how
															the system is connected - wireless or ethernet""")
args = parser.parse_args(sys.argv[1:])

ip,bcast = getServerIp(args.connection)
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
# Bind the socket to the port
server_address = (ip, port)

print >>sys.stderr, 'starting up on %s port %s' % server_address
sock.bind(server_address)

t0 = time()

while True:
	t1 = time()
	if(t1 - t0 >= AUTHENTICATION_INTERVAL):
		t0 = t1
		AUTHENTICATION_INTERVAL = 300
		askAuthentication(sock, bcast)
		print registeredDevices

	sock.settimeout(AUTHENTICATION_INTERVAL)
	try:
		data, address = sock.recvfrom(4096)
		print data, address
		decisionBox(sock, address, data)
	except:
		pass
	sock.settimeout(None)

sock.close()
        