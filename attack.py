import argparse
from time import time

import netifaces as ni
import socket
import sys
from subprocess import call

listenPort = 8888

def getMyIp(connection):
	try:
		ip = ni.ifaddresses(connection)[2][0]['addr']
		bcast = ni.ifaddresses(connection)[2][0]['broadcast']
		netmask = ni.ifaddresses(connection)[2][0]['netmask']
		return ip,bcast,netmask
	except:
		print "Can't get IP for this interface. Make sure with ifconfig"
		sys.exit(0)

def getIpConstant(ip,nmask):
	iplist = ip.split('.')
	nmasklist = nmask.split('.')
	constantip = ''
	for i in xrange(len(nmasklist)):
		if nmasklist[i] == '255':
			constantip += iplist[i]
			constantip += '.'
		elif nmasklist[i] == '0':
			break
	return constantip

parser = argparse.ArgumentParser()
parser.add_argument('--connection', choices=['wlan0', 'eth0'], default='wlan0', help="""Describes how
															the system is connected - wireless or ethernet""")
args = parser.parse_args(sys.argv[1:])

ip,bcast,nmask = getMyIp(args.connection)
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
# Bind the socket to the port
my_address = (bcast, listenPort)

print >>sys.stderr, 'starting up on %s port %s' % my_address
sock.bind(my_address)
dest = (bcast,listenPort)
# sock.sendto('xyz',dest)
# print "Data sent to  %s, %s" %(bcast, str(listenPort))
data, server_address = sock.recvfrom(4096)
print "Data = %s , Server_address = %s" %(data, server_address)
constantip = getIpConstant(ip, nmask)

while True:
	data = raw_input("Enter data to be sent: ")
	if data == 'q':
		break

	for i in xrange(1,255):
		dest_ip = constantip + str(i)
		if dest_ip != server_address[0]:
			command = 'sudo sendip -p ipv4 -is %s -p udp -us 8080 -ud %s -d %s -v %s' %(server_address[0], str(listenPort), data, dest_ip)
			print command
			call(command.split())
sock.close()
        