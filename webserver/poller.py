import select
import socket
import sys
import time
import os

from HTTPRequest import HTTPRequest
from clientdata import ClientData
from time import strftime

class Poller:
	""" Polling server """
	def __init__(self, port, debug):
		self.host = ""
		self.port = port
		self.debug = debug
		self.clients = {}
		self.clientDatas = {}
		self.hosts = {}
		self.medias = {}
		self.params = {}
		self.configure()
		self.open_socket()
		self.size = 10240
		
	def configure(self):
		""" Load configuration from web.conf"""
		config = open("web.conf")
		configLines = config.readlines()
		config.close()

		for line in configLines:
			if line.startswith("host"):
				words = line.split()
				if not words[2].startswith(os.sep):
					words[2] = os.path.join(os.path.dirname(os.path.abspath(__file__)), words[2])
				self.hosts[words[1]] = words[2]
			elif line.startswith("media"):
				words = line.split()
				self.medias[words[1]] = words[2]
			elif line.startswith("parameter"):
				words = line.split()
				self.params[words[1]] = int(words[2])
				
		
	def open_socket(self):
		""" Setup the socket for incoming clients """
		try:
			self.server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
			self.server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR,1)
			self.server.bind((self.host,self.port))
			self.server.listen(5)
		except socket.error, (value,message):
			if self.server:
				self.server.close()
			print "Could not open socket: " + message
			sys.exit(1)

	def run(self):
		""" Use poll() to handle each incoming client."""
		print "Server running on port " + str(self.port)
		self.poller = select.epoll()
		self.pollmask = select.EPOLLIN | select.EPOLLHUP | select.EPOLLERR
		self.poller.register(self.server,self.pollmask)
		timeCount = 0
		while True:
			# poll sockets
			startTime = time.time()
			
			try:
				fileDescList = self.poller.poll(timeout=self.params['timeout'])
			except:
				return
			
			timeCount += (time.time() - startTime)
				
			for (fileDesc,event) in fileDescList:
				# handle errors
				if event & (select.POLLHUP | select.POLLERR):
					self.handleError(fileDesc)
					continue
				# handle the server socket
				if fileDesc == self.server.fileno():
					self.handleServer()
					continue
				# handle client socket
				self.clientDatas[fileDesc].lastUse = time.time()
				result = self.handleClient(fileDesc)
			
			if timeCount > self.params['timeout']:
				timeCount = 0
				self.checkAndDestroy()
				

	def checkAndDestroy(self):
		for fileDesc, client in self.clientDatas.items():
			if self.clients.get(fileDesc, None):	
				if time.time() - client.lastUse > self.params['timeout']:
					self.clients[fileDesc].close()
					del self.clients[fileDesc]
					del self.clientDatas[fileDesc]

	def handleError(self,fileDesc):
		self.poller.unregister(fileDesc)
		if fileDesc == self.server.fileno():
			# recreate server socket
			self.server.close()
			self.open_socket()
			self.poller.register(self.server,self.pollmask)
		else:
			# close the socket
			self.clients[fileDesc].close()
			del self.clients[fileDesc]

	def handleServer(self):
		(client,address) = self.server.accept()
		self.clients[client.fileno()] = client
		self.poller.register(client.fileno(),self.pollmask)
		self.clientDatas[client.fileno()] = ClientData(time.time())

	def handleClient(self,fileDesc):
		data = self.clients[fileDesc].recv(self.size)
		if data:
		
			if self.debug: print "[DATA]\n" + data + "\n"
			self.clientDatas[fileDesc].cache += data
			if self.validRequestInCache(fileDesc):
				requestData = self.removeRequestFromCache(fileDesc)
				print "[REQUEST DATA]\n" + requestData + "\n"
				request = HTTPRequest(requestData)
				
				if not request.command:
					response = self.create400Response()
					if self.debug:
						print "*************** Sending Response ***************\n" + response + "************************************************"
					self.clients[fileDesc].send(response)
					return
				
				elif request.command != 'GET':
					response = self.create501Response()
					if self.debug:
						print "*************** Sending Response ***************\n" + response + "************************************************"
					self.clients[fileDesc].send(response)
					return

				elif request.command == 'GET':
					if request.headers['host'].find(":"):
						host = request.headers['host'][:request.headers['host'].find(":")]
					host = request.headers['host']
					hostPath = self.hosts[host]
					
					if self.debug:
						print "[Hostpath] " + hostPath
					
					if not hostPath:
						hostPath = self.hosts['default']
						
					if not request.path:
						request.path = "/index.html"
						
					if request.path == "/":
						request.path = "/index.html"
					
					filePath = hostPath + request.path
					
					if not os.path.isfile(filePath):
						response = self.create404Response()
						if self.debug:
							print "*************** Sending Response ***************\n" + response + "************************************************"						
						self.clients[fileDesc].send(response)
						return
					
					if not os.access(filePath, os.R_OK):
						response = self.create403Response()
						if self.debug:
							print "*************** Sending Response ***************\n" + response + "\n************************************************"
						self.clients[fileDesc].send(response)
						return
						
					reqFile = open(filePath);
					response = self.create200Response(reqFile)
					if self.debug:
						print "*************** Sending Response ***************\n" + response + "File Contentss go here\n ************************************************"
					self.clients[fileDesc].send(response)
										
					while True:
						filePiece = reqFile.read(self.size)
						if filePiece == "":
							break
						response += filePiece					
						self.clients[fileDesc].send(filePiece)

#					self.clients[fileDesc].send(response)
				
		else:
			self.poller.unregister(fileDesc)
			self.clients[fileDesc].close()
			del self.clients[fileDesc]		
		
	def create200Response(self, goodFile):
		ext = goodFile.name.split('.')[-1]
		if not self.medias[ext]:
			ext = 'txt'
			
		conType = self.medias[ext]
		size = int(os.path.getsize(goodFile.name))
		mTime = strftime('%a, %d %b %Y %H:%M:%S %Z', time.localtime(os.path.getmtime(goodFile.name)))		
	
		return 'HTTP/1.1 200 OK\r\n' + 'Date: ' + strftime('%a, %d %b %Y %H:%M:%S %Z') + '\r\n' + 'Server: CS360_WebServer/0.0.0.0.1\r\n' + 'Content-Type: ' + conType + '\r\n' + 'Content-Length: ' + str(size)+ '\r\n' + 'Last-Modified: ' + mTime + '\r\n\r\n'	
	
	def create500Response(self):
		return 'HTTP/1.1 500 Internal Server Error\r\n' + 'Date: ' + strftime('%a, %d %b %Y %H:%M:%S %Z') + '\r\n' + 'Server: CS360_WebServer/0.0.0.0.1\r\n' + 'Content-Type: text/html\r\n' + 'Content-Length: 60\r\n' + 'Last-Modified: ' + strftime('%a, %d %b %Y %H:%M:%S %Z') + '\r\n\r\n' + '<html><body><h1>Internal Server Error 500</h1></body></html>\r\n'
		
	def create501Response(self):
		return 'HTTP/1.1 501 Not Implemented\r\n' + 'Date: ' + strftime('%a, %d %b %Y %H:%M:%S %Z') + '\r\n' + 'Server: CS360_WebServer/0.0.0.0.1\r\n' + 'Content-Type: text/html\r\n' + 'Content-Length: 50\r\n' + 'Last-Modified: ' + strftime('%a, %d %b %Y %H:%M:%S %Z') + '\r\n\r\n' + '<html><body><h1>Not Implemented</h1></body></html>\r\n'
		
	def create404Response(self):
		return 'HTTP/1.1 404 Not Found\r\n' + 'Date: ' + strftime('%a, %d %b %Y %H:%M:%S %Z') + '\r\n' + 'Server: CS360_WebServer/0.0.0.0.1\r\n' + 'Content-Type: text/html\r\n' + 'Content-Length: 44\r\n' + 'Last-Modified: ' + strftime('%a, %d %b %Y %H:%M:%S %Z') + '\r\n\r\n' + '<html><body><h1>Error 404</h1></body></html>\r\n'
		
	def create403Response(self):
		return 'HTTP/1.1 403 Forbidden\r\n' + 'Date: ' + strftime('%a, %d %b %Y %H:%M:%S %Z') + '\r\n' + 'Server: CS360_WebServer/0.0.0.0.1\r\n' + 'Content-Type: text/html\r\n' + 'Content-Length: 64\r\n' + 'Last-Modified: ' + strftime('%a, %d %b %Y %H:%M:%S %Z') + '\r\n\r\n' + '<html><body><h1>403 Forbidden: You\'re a n00b!</h1></body></html>\r\n'
		
	def create400Response(self):
		return 'HTTP/1.1 400 Bad Request\r\n' + 'Date: ' + strftime('%a, %d %b %Y %H:%M:%S %Z') + '\r\n' + 'Server: CS360_WebServer/0.0.0.0.1\r\n' + 'Content-Type: text/html\r\n' + 'Content-Length: 50\r\n' + 'Last-Modified: ' + strftime('%a, %d %b %Y %H:%M:%S %Z') + '\r\n\r\n' + '<html><body><h1>400 Bad Request</h1></body></html>\r\n'
		
		
	def validRequestInCache(self, fileDesc):
		return "\r\n\r\n" in self.clientDatas[fileDesc].cache
	
	def removeRequestFromCache(self, fileDesc):
		endIndex = self.clientDatas[fileDesc].cache.find("\r\n\r\n")
		request = self.clientDatas[fileDesc].cache[:endIndex + 4]
		self.clientDatas[fileDesc].cache = self.clientDatas[fileDesc].cache[endIndex + 4:]
		return request
	
	
	
