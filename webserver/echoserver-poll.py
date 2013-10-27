"""
A TCP echo server that handles multiple clients with polling.  Typing
Control-C will quit the server.
"""

import argparse

from poller import Poller

class Main:
    """ Parse command line options and perform the download. """
    def __init__(self):
        self.parse_arguments()

    def parse_arguments(self):
        ''' parse arguments, which include '-p' for port '''
        parser = argparse.ArgumentParser(prog='web.py', description='A simple web server that implements some of HTTP 1.1, and can handle multiple clients at a time.', add_help=True)
		parser.add_argument('-d', '--debug', action='store_true', dest=debug, help='print out debug statements')
        parser.add_argument('-p', '--port', type=int, action='store', help='port the server will bind to',default=8080)
        self.args = parser.parse_args()

    def run(self):
    	print debug
        p = Poller(self.args.port)
        p.run()

if __name__ == "__main__":
    m = Main()
    m.parse_arguments()
    try:
        m.run()
    except KeyboardInterrupt:
        pass
