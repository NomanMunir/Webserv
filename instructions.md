```markdown
# RFC

## 1.1 Message Format:
```
    HTTP-message   = start-line CRLF // start-line = request-line / status-line
                     *( field-line CRLF )
                     CRLF
                     [ message-body ]

## 1.2. Message Parsing
- **LF**: `\n`
- **CR**: `\r`
1. Read the start-line, then each header field line into a hash table until an empty line.
2. Check if a message body is expected and read it if indicated.
3. Parse the message as a sequence of bytes in an encoding compatible with US-ASCII.

4. Directly parsing as Unicode can cause security vulnerabilities.
5. Server  is expecting to receive and parse a request-line SHOULD ignore at least one empty line (CRLF) received prior to the request-line.
6. CRLF is the standard line terminator in HTTP, but recipients may accept a single LF as terminator. 
7. Bare CRs are not allowed as terminator in protocol elements by senders, and recipients should treat them as invalid or replace them with spaces.
8. A sender MUST NOT send whitespace between the start-line and the first header field.
9. A recipient that receives whitespace between the start-line and the first header field MUST either reject the message as invalid
10. Server SHOULD respond with a 400 (Bad Request) response and close the connection if grammer is not correct

## How to use octet-parsing:

1. Read the HTTP message as a sequence of bytes. This means that the data is handled as raw binary data.

2. Use specific byte sequences to identify the boundaries of different components of the HTTP message (e.g., start-line, headers, and body). For instance, the CRLF sequence (\r\n or bytes 0x0D 0x0A) is used to mark the end of each line.

3. Extract the start-line, headers, and body as byte sequences. At this stage, the data is still in its raw form and has not been converted into strings.

4. Converting Byte Sequences to Strings: After identifying and extracting the individual components, convert these byte sequences into strings using the correct encoding (e.g., UTF-8). This step is done after ensuring the boundaries are correctly identified.


## 2.1. HTTP Version
The version of an HTTP/1.x message is indicated by an HTTP-version field in the start-line. HTTP-version is case-sensitive.

  HTTP-version  = HTTP-name "/" DIGIT "." DIGIT
  HTTP-name     = %s"HTTP"

1. HTTP/1.1 can gracefully downgrade to a format that can be understood by HTTP/1.0 recipients.

## 3.1 Request Line:

- ## Syntax:
A request-line begins with a method token, followed by a single space (SP), the request-target, and another single space (SP), and ends with the protocol version.

  request-line   = method SP request-target SP HTTP-version

## Method:          
    Request method to be performed on the target resource. case-sensitive, can define new method

- ## Parsing:
Parse on whitespace-delimited word boundaries and, aside from the CRLF terminator, treat any form of whitespace as the SP separator while ignoring preceding or trailing whitespace; such whitespace includes one or more of the following octets: SP, HTAB, VT (%x0B), FF (%x0C), or bare CR. 

- ## Length:
HTTP does not place a predefined limit on the length of a request-line. 
In practice, It is RECOMMENDED that all HTTP senders and recipients support, at a minimum, request-line lengths of 8000 octets.

- Method longer than any that it implements SHOULD respond with a 501 (Not Implemented) status code. 
- Request-target longer than any URI it wishes to parse MUST respond with a 414 (URI Too Long) status code 

## 3.2 Request Target: 
Target resource upon which to apply the request

- ## Forms:
There are four distinct formats for the request-target, depending on both the method being requested and whether the request is to a proxy.
request-target = origin-form
                / absolute-form
                / authority-form
                / asterisk-form

- ## Syntax:
1. No whitespace is allowed in the request-target.
2. Invalid request-line: 400 (Bad Request) error or a 301 (Moved Permanently) redirect with the request-target properly encoded. A recipient // SHOULD NOT attempt to autocorrect
3. A client MUST send a Host header field  in all HTTP/1.1 request messages. 
4. If the target URI includes an authority component, then a client MUST send a field value for Host that is identical to that authority component, excluding any userinfo subcomponent and its "@" delimiter. 
5. If the authority component is missing or undefined for the target URI, then a client MUST send a Host header field with an empty field value.
6. A server MUST respond with a 400 (Bad Request) status code to any HTTP/1.1 request message that lacks a Host header field and to any request message that contains more than one Host header field line or a Host header field with an invalid field value.

- ## Origin-form:

- ## Syntax:
  origin-form    = absolute-path [ "?" query ]
- Client MUST send only the absolute path and query components of the target URI as the request-target. 
- If the target URI's path component is empty, the client MUST send "/" as the path. 
- A Host header field is also sent.

- ## Example
  http://www.example.org/where?q=now
    Server would open a TCP connection to port 80 of the host "www.example.org" and send the lines:
    GET /where?q=now HTTP/1.1
    Host: www.example.org
    followed by the remainder of the request message.

- ## Absolute-form:
  Request to a proxy, a client MUST send the target URI in "absolute-form" as the request-target.

- ## Syntax:
  absolute-form  = absolute-URI

- ## Example:

GET http://www.example.org/pub/WWW/TheProject.html HTTP/1.1

- A client MUST send a Host header field in an HTTP/1.1 request even if the request-target is in the absolute-form, since this allows the Host information to be forwarded through ancient HTTP/1.0 proxies that might not have implemented Host.

- ## Cases:
Proxy:
- Ignore the received Host header field and  replace it with the host information of the request-target. 
- A proxy that forwards such a request MUST generate a new Host field value based on the received request-target rather than forward the received Host field value.

Origin Server:
- Ignore the received Host header field and use the host information of the request-target. 
- Note that if the request-target does not have an authority component, an empty Host header will be sent.


- ## Authority-form:
- Only used for CONNECT requests. A CONNECT request is a method used in the HTTP protocol to establish a tunnel through a proxy server. 

- ## Syntax:
It consists of only the uri-host and port number of the tunnel destination, separated by a colon (":").

  authority-form = uri-host ":" port

- ## Example:
CONNECT www.example.com:80 HTTP/1.1
Host: www.example.com
Request Method: CONNECT
Request-Target: www.example.com:80 (authority-form)
HTTP Version: HTTP/1.1
Host Header: Specifies the host (www.example.com) to which the tunnel connection is requested.


- ## Asterisk-form:
- Used for a server-wide OPTIONS request Server responds with information about the server's capabilities and supported HTTP methods for all resources.

- ## Syntax:
  asterisk-form  = "*"
  OPTIONS * HTTP/1.1
Client requests OPTIONS for the server as a whole, as opposed to a specific named resource of that server.


If a proxy receives an OPTIONS request with an absolute-form of request-target in which the URI has an empty path and no query component, then the last proxy on the request chain MUST send a request-target of "*" when it forwards the request to the indicated origin server.

For example, the request

OPTIONS http://www.example.org:8001 HTTP/1.1
would be forwarded by the final proxy as

OPTIONS * HTTP/1.1
Host: www.example.org:8001
after connecting to port 8001 of host "www.example.org".

