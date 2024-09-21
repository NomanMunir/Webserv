
import http.client
import time

chunk1 = "This is the first chunk"
chunk2 = "This is the second chunk"

conn = http.client.HTTPConnection('localhost:8080')
conn.connect()
conn.putrequest('POST', '/upload')
conn.putheader('Transfer-Encoding', 'chunked')
conn.putheader('Content-Type', 'application/x-www-form-urlencoded')
conn.endheaders()

# Send first chunk size
conn.send(("%s\r\n" % hex(len(chunk1))[2:]).encode('utf-8'))
# Send first chunk data
conn.send(("%s\r\n" % chunk1).encode('utf-8'))

time.sleep(1)

# Send second chunk size
conn.send(("%s\r\n" % hex(len(chunk2))[2:]).encode('utf-8'))
# Send second chunk data
conn.send(("%s\r\n" % chunk2).encode('utf-8'))

time.sleep(1)

# Send last chunk (size zero)
conn.send(("0\r\n\r\n").encode('utf-8'))

# Get the response
r = conn.getresponse()
print(r.status, r.reason)
print(r.read().decode('utf-8'))
