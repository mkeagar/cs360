"""
A Web server that handles multiple clients using polling.  It implements some, but not all of HTTP/1.1.
Typing Control-C will quit the server.
"""

import argparse

from poller import Poller

class WebServer:
	""" Parse command line options and perform the start the server. """
	def __init__(self):
		self.parse_arguments()

	def parse_arguments(self):
		''' parse arguments, which include '-p' for port '''
		parser = argparse.ArgumentParser(prog='web.py', description='A simple web server that implements some of HTTP 1.1, and can handle multiple clients at a time.', add_help=True)
		parser.add_argument('-p', '--port', type=int, action='store', help='port the server will bind to',default=8080)
		parser.add_argument('-d', '--debug', action='store_true', help='print out debug statements')
		self.args = parser.parse_args()

	def run(self):
		p = Poller(self.args.port, self.args.debug)
		p.run()

if __name__ == "__main__":
	m = WebServer()
	try:
		m.run()
	except KeyboardInterrupt:
		pass
