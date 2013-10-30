import argparse
import os
import requests
import threading
import time

''' CS 360 - Fall 2013 - Lab 2 - Download Accelerator - will download a specified URL using a specified number of threads, or a default number of threads. '''
class DownloadAccelerator:
	def __init__(self):
		''' Parse command line arguments to get the URL to download and if specified, the number of threads to use to download it.'''
		self.args = None;
		self.parse_arguments()
		
	def parse_arguments(self):
		''' Parse arguments, which include -n to get the number of threads to use, and a URL, to get the file to download.'''
		parser = argparse.ArgumentParser(prog='Download Accelerator', description='A script that will download a file in parallel using mulitple threads if a number is specified.', add_help=True)
		parser.add_argument('-n', '--threads', type=int, action='store', help='Specify the number of threads to use to download the URL.', default=1)
		parser.add_argument('URL', type=str, action='store', help='Specify the URL for the file to download', default=None)
		args = parser.parse_args()
		self.numThreads = args.threads
		self.url = args.URL
		
	def download(self):
		''' Download the file specified by the URL using the specified number of threads, or one thread if no number was specified.'''
		# append "index.html" if needed.
		suffix = "/"
		if self.url.endswith(suffix):
			self.url += 'index.html'
		
		# setup download location and size variables for dividing the data among our threads
		output = self.url.split('/')[-1].strip()
		
		timeStart = time.time() # The time we start trying to get the file
		r = requests.head(self.url)
		
		startByte = 0;
		partSize = 0;
		endbyte = 0;
		
		if not r.headers['content-length']:
			self.numThreads = 1
			startByte = -1
			endByte = -1
			partSize = -1
		else:
			partSize = int(r.headers['content-length']) / self.numThreads
			
		if not startByte == -1:
			endByte = startByte + partSize
		
		# create our thread array with the number of threads specified.
		threads = []
		for i in range(0,self.numThreads):
			if i == self.numThreads-1 and r.headers['content-length']:
				endByte = int(r.headers['content-length'])
			d = DownThread(self.url, startByte, endByte)
			threads.append(d)
			startByte = endByte + 1
			endByte = startByte + partSize
			
		# start our threads downloading!
		for t in threads:
			t.start()
		
		f = open(output, 'wb')
		
		for t in threads:
			t.join()
			f.write(t.content)
		f.close()
		timeEnd = time.time() - timeStart
		statinfo = os.stat(output)
		print self.url + ' ' + str(self.numThreads) + ' ' + str(statinfo.st_size) + ' ' + str(timeEnd)
		
class DownThread(threading.Thread):
	def __init__(self, url, index1, index2):
		self.url = url
		self.startByte = index1
		self.endByte = index2
		self.content = None
		
		threading.Thread.__init__(self)
		self._content_consumed = False

	def run(self):
		if self.startByte == -1:
			r = requests.get(self.url, stream=True, headers={'accept-encoding':''})
		else:
			r = requests.get(self.url, stream=True, headers={'accept-encoding':'', 'range':'bytes='+str(self.startByte) + '-' + str(self.endByte)})

		self.content = r.content

if __name__ == '__main__':
	d = DownloadAccelerator()
	d.download()
