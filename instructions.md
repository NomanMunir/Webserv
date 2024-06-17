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

## 3.3 Reconstructing the Target URI
  - Target URI is to identify the target resource of an HTTP request.
  - Identify the scheme (http, https), authority, and path components of the target URI.
  - The target URI is reconstructed by concatenating the request-target (origin-form) with the Host header field value.

- Cases:
  1. Origin-form
    - The authority component is the same as the Host header field value. If there is no Host header field or invalid field value, the authority component is undefined.
    - The path component is the same as the request-target.
  - Example:
    GET /where?q=now HTTP/1.1
    Host: www.example.org
  - Target URI: http://www.example.org/where?q=now

  2. Absolute-form
    - The authority component is the same as the Host header field value. If there is no Host header field or invalid field value, the authority component is undefined.
    - The target URI is the request-target.
    - The path component is the same as the request-target.
  - Example:
    GET http://www.example.org/pub/WWW/TheProject.html HTTP/1.1
    Host: www.example.org
  - Target URI: http://www.example.org/pub/WWW/TheProject.html

  3. Authority-form
    - The authority component is the same as the request-target.
    - The target URI is the request-target.
    - The path component is empty.
    - Example:
    CONNECT www.example.com:80 HTTP/1.1
    Host: www.example.com
  - Target URI: http://www.example.com:80

  4. Asterisk-form
    - The authority component is the same as the Host header field value. If there is no Host header field or invalid field value, the authority component is undefined.
    - The path component is empty.
  - Example:
    OPTIONS * HTTP/1.1
    Host: www.example.org:8001
  - Target URI: http://www.example.org:8001

- The target URI is reconstructed by concatenating the scheme, "://", authority, and path component.
- The scheme is "http" for HTTP requests and "https" for HTTPS requests.
- If the authority component is empty and it's URI scheme requires a non-empty authority component, the request is malformed and the server MUST respond with a 400 (Bad Request) status code.

## 4.1. Status Line:
  The status line in an HTTP response message is formatted according to specific rules defined by the HTTP specification. Here’s a breakdown of the structure and components of the status line.
- ## Syntax:
A status-line consists of the protocol version, a space (SP), the status code, another space, a reason phrase, and ends with CRLF.

  status-line    = HTTP-version SP status-code SP reason-phrase
- ## Parsing:
  - Same as request-line parsing.
  - The status code is a 3-digit integer result code of the server's attempt to understand and satisfy the request.
  - The reason phrase is intended to give a short textual description of the status code.
  - The reason phrase is optional and can be empty.
  - Status-code:
    - 1xx: Informational - Request received, continuing process
    - 2xx: Success - The action was successfully received, understood, and accepted
    - 3xx: Redirection - Further action must be taken to complete the request
    - 4xx: Client Error - The request contains bad syntax or cannot be fulfilled
    - 5xx: Server Error - The server failed to fulfill an apparently valid request

## 5 Field Syntax:
  - Each field line consists of a case-insensitive field name followed by a colon (":"), optional leading whitespace, the field line value, and optional trailing whitespace.

    - field-line   = field-name ":" OWS field-value OWS

  ## 5.1. Field Line Parsing:
    - Field names are case-insensitive.
    - Field values are case-sensitive.
  - Rules:
    1. No whitespace is allowed between the field-name and colon. Server response with error 400 (Bad Request) if found.
    2. Whiespace is allowed between the colon and the field-value.
    3. Leading and trailing whitespace in field values are ignored.

  ## 5.2. Obsolete Line Folding:
    - Line folding is the process of breaking a long line into multiple lines by inserting a CRLF and at least one SP or HTAB.
    # Syntax:
      obs-fold     = OWS CRLF RWS
                    ; obsolete line folding

    - Line folding is obsolete and MUST NOT be used by senders unless the message is within the "message/http".
    - Server must reject the massage with 400 (Bad Request) if found unless the message is within the "message/http".
    - Proxy or gateway must discard the message and replace it with a 502 (Bad Gateway) response or remove the obs-fold before forwarding the message.
## 6. Message Body:
  - Consists of any sequence of octets. It carries the actual content of the request or response, unless modified by transfer encodings.
  - Request:
    1. Presence of a message body is indicated by either a Content-Length header field or Transfer-Encoding header field.
    2. Transfer-Encoding takes precedence over Content-Length.
  - Response:
    1. Presence of a message body depends on both the method of the original request and the status code of the response.
   - Example:
      - For responses to GET or HEAD requests:
      - If the response status is 200 OK, Message body is send.
      - If the status is 204 No Content or 304 Not Modified, the response must not include a message body.
    
    ## 6.1 Transfer-Encoding:
      - Specifies the transfer codings that have been (or will be) applied to the message body.
      - Insure Proper framing and transmission of dynamically generated or unknown-length content through chunked encoding.
      - Distinguish between transfer encodings (encodings applied during the transmission process, such as chunked transfer) and content encodings (encodings applied to the representation of the content itself, such as compression).
      - Additional endcdings can be applied to the message body, and additional Transfer-Encoding fields can be added to the message.
      - In HEAD request (request of the header only without the mesage content) and in 304 (Not Modified) response, which both dont send message content, transfer-Encoding MAY be sent.
      - Server must not include transfer-encoding header in case of a status code of 1xx (Informational), 204 (No Content), and any 2xx (Successful) response to a CONNECT request
      - If transfer-encoding is unknown to the server, server MUST respond with 501 (Not Implemented).
        - Servers and clients that only support HTTP/1.0 typically do not handle Transfer-Encoding correctly and may treat messages as malformed if Transfer-Encoding is present.
        - A client should not send a request with Transfer-Encoding unless it knows the server supports HTTP/1.1 or later.
        - Similarly, a server should not send a response with Transfer-Encoding unless the client's request indicates HTTP/1.1 compatibility.
        - If multiple encodings are applied, chunked encoding should be the final one to ensure proper framing.
        - A server/client must not apply chunked encoding more than once to a message body.

      - Security Considerations:
      - Mixing Transfer-Encoding with Content-Length can lead to security vulnerabilities. Servers should handle such cases cautiously by closing the connection after responding or reject them outright.
       Server/ client recieving HTTP 1.0 that has transfer-encoding header, it is considered faultly and the connection is closed afterward. 

    ## 6.2 Content Length:
      - The Content-Length header field indicates the size of the message body in octets.
      - The Content-Length field value is a decimal number.
      - Purpose:
        1. Framing Information allowing the recipient to know where the body ends.
        2. Size of the Representation: For messages that do not include a body (e.g., HEAD responses), the Content-Length can indicate the size of the selected message.

    ## 6.3. Message Body Length:
    Cases:
    1. Responses to HEAD Requests and Specific Status Codes (1xx, 204, 304): 
      - These responses are terminated by the first empty line after the header fields and do not contain a message body or trailer section.

    2. 2xx Responses to CONNECT Requests:
      - These responses indicate that the connection becomes a tunnel immediately after the header fields, and any Content-Length or Transfer-Encoding headers should be ignored.

    3. Transfer-Encoding vs. Content-Length:
      - If both Transfer-Encoding and Content-Length headers are present, Transfer-Encoding takes precedence. Intermediaries must remove the Content-Length header or consider it as error.

    4. Chunked Transfer-Encoding:
      - If the Transfer-Encoding header field ends with the chunked transfer coding, the message body length is determined by reading the chunked data until the end of the chunked transfer coding.
      - If chunked is not the final encoding:
        - In RESPONSE, the length is determined by reading until the server closes the connection.
        - In REQUESR, the server must respond with a 400 (Bad Request) status code and close the connection.

    5. Invalid Content-Length:
      - Request: The server must respond with 400 (Bad Request) and close the connection.
      - Response: The client must discard the message body and close the connection.
      - Response to proxy: The proxy must close the connection and discard the message body and send a 502 (Bad Gateway) response.

    6. Valid Content-Length:
      - It defines the expected message body length in octets. If the connection closes before receiving the indicated length, the message is considered incomplete.
    
    7. Message Body Length Determination:
      - If none of the above conditions apply to a request message, the message body length is zero.
      - For responses, the message body length is determined by the octets received before the server closes the connection. However, this method can lead to ambiguities if the connection is interrupted or length is reached, so length or encoding delimitations (not close-delimitation) are preferred.

    - A server may reject requests with a message body but without Content-Length by responding with 411 (Length Required).
    - Clients should send a Content-Length header if the message body length is known to avoid issues with some services that might reject chunked transfer coding.
    - Clients must either include a Content-Length header or use chunked transfer coding for messages with a body, ensuring the server can handle HTTP/1.1 or later.
    
    8. Extra Data After Final Response:
      - If extra data remains after the final response, a user agent may discard it or attempt to determine if it belongs to the prior message.
      - Clients must not process, cache, or forward such data to avoid cache poisoning.
