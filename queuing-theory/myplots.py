import optparse
import sys
import os

import matplotlib
matplotlib.use('Agg')
from pylab import *

# Class that parses a file and plots several graphs
class Plotter:
	def __init__(self):
		self.x = [ 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 0.99]
		self.xPercent = []
		for num in self.x:
			self.xPercent.append(str(num) + "%")
		
		self.loads = [78, 156, 234, 312, 390, 468, 546, 624, 702, 780]
		self.webserver = []
		self.lighttpd = []
		self.nginx = []
		self.prefixes = ["load_webserver_", "load_lighttpd_", "load_nginx_"]
		self.ext = ".txt"
		
		for prefix in self.prefixes:
			vals = []
			for load in self.loads:
				lines = open(prefix + str(load) +self.ext)
				things = []
				for line in lines:
					thing = float(line.split()[-1])
					if thing < 0.1:
						things.append(float(line.split()[-1]))
				vals.append(things)
			if prefix=="load_webserver_":
				self.webserver = vals
			elif prefix=="load_lighttpd_":
				self.lighttpd = vals
			elif prefix=="load_nginx_":
				self.nginx = vals
				
	def webserverPlot(self):
		""" Create a line graph of an equation. """
		clf()
		x = np.arange(0,0.99,0.01)
		plot(x, 1/((1/0.00127831935883)*(1-x)), label="webserver")	# webserver
		xlabel('Utilization')
		ylabel('Time in System')
		legend(loc=0)
		savefig('part2_webserver.png')
		
	def lighttpdPlot(self):
		""" Create a line graph of an equation. """
		clf()
		x = np.arange(0,0.99,0.01)
		plot(x, 1/((1/0.0015021276474)*(1-x)), label="lighttpd")	# lighttpd
		xlabel('Utilization')
		ylabel('Time in System')
		legend(loc=0)
		savefig('part2_lighttpd.png')
		
	def nginxPlot(self):
		""" Create a line graph of an equation. """
		clf()
		x = np.arange(0,0.99,0.01)
		plot(x, 1/((1/0.00143876075745)*(1-x)), label="nginx")	# nginx
		xlabel('Utilization')
		ylabel('Time in System')
		legend(loc=0)
		savefig('part2_nginx.png')
				
				
				
	def combinedEquationPlot(self):
		""" Create a line graph of an equation. """
		clf()
		x = np.arange(0,0.99,0.01)
		plot(x, 1/((1/0.00127831935883)*(1-x)), label="webserver")	# webserver
		plot(x, 1/((1/0.0015021276474)*(1-x)), label="lighttpd")	# lighttpd
		plot(x, 1/((1/0.00143876075745)*(1-x)), label="nginx")	# nginx
		xlabel('Utilization')
		ylabel('Time in System')
		legend(loc=0)
		savefig('part2.png')

	def combinedWebserverBoxPlot(self):
		""" Boxplot for webserver """
		clf()
		x = np.arange(0,0.99,0.01)
		plot(x, 1/((1/0.00127831935883)*(1-x)), label="webserver", color="Blue")	# webserver
		boxplot(self.webserver, positions=self.x, widths=0.05)
		xticks(self.x, rotation=30)
		xlabel('Utilization')
		ylabel('Time in System')
		savefig('webserverBoxplot.png')
		
	def combinedLighttpdBoxPlot(self):
		""" Boxplot for lighttpd """
		clf()
		x = np.arange(0,0.99,0.01)
		plot(x, 1/((1/0.0015021276474)*(1-x)), label="lighttpd", color="Lime")	# lighttpd
		boxplot(self.lighttpd, positions=self.x, widths=0.05)
		xticks(self.x, rotation=30)
		xlabel('Utilization')
		ylabel('Time in System')
		savefig('lighttpdBoxplot.png')
		
	def combinedNginxBoxPlot(self):
		""" Boxplot for nginx """
		clf()
		x = np.arange(0,0.99,0.01)
		plot(x, 1/((1/0.00143876075745)*(1-x)), label="nginx", color="Red")	# nginx
		boxplot(self.nginx, positions=self.x, widths=0.05)
		xticks(self.x, rotation=30)
		xlabel('Utilization')
		ylabel('Time in System')
		savefig('nginxBoxplot.png')

if __name__ == '__main__':
	p = Plotter()
	p.webserverPlot()
	p.lighttpdPlot()
	p.nginxPlot()
	p.combinedEquationPlot()
	p.combinedWebserverBoxPlot()
	p.combinedLighttpdBoxPlot()
	p.combinedNginxBoxPlot()
