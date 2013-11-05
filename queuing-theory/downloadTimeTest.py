import time
import requests

url = "http://162.219.3.125:4000/index.lighttpd.html"
file_name = "times_nginx.txt"
f = open(file_name, 'wb')
total = 0
for x in range(1, 101):
	timestart = time.time()
	r = requests.get(url)
	timeend = time.time()
	currenttime = timeend - timestart
	total += currenttime
	f.write(str(currenttime) + "\n")

avg = total/100
f.write("\n" + str(avg) + "\n")
f.close()
  
