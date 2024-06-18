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

## 7. Transfer Codings
    Allow a server to send a response in a series of chunks, rather than as a single continuous block of data. Useful when the total size of the response content is unknown at the start of transmission.

  - **Syntax**:
    ```
    chunked-body   = *chunk
                   last-chunk
                   trailer-section
                   CRLF

    chunk          = chunk-size [ chunk-ext ] CRLF
                    chunk-data CRLF
    chunk-size     = 1*HEXDIG
    last-chunk     = 1*("0") [ chunk-ext ] CRLF

    chunk-data     = 1*OCTET ; a sequence of chunk-size octets

    ```
  #### Structure of Chunked Transfer Coding

    1. **Chunked Body**:
      - The chunked body consists of a series of chunks, a final chunk (last-chunk), an optional trailer section, and ends with a CRLF (carriage return and line feed).
      - **Syntax**:
        ```
        chunked-body = *chunk
                        last-chunk
                        trailer-section
                        CRLF
        ```

    2. **Chunk**:
      - Each chunk starts with a chunk-size in hexadecimal, optionally followed by chunk extensions, then a CRLF, the chunk data itself, and another CRLF.
      - **Syntax**:
        ```
        chunk = chunk-size [ chunk-ext ] CRLF
                chunk-data CRLF
        ```
      - **chunk-size**: A string of hexadecimal digits indicating the size of the chunk-data in octets (bytes).
      - **chunk-data**: The actual data of the chunk, the length of which is specified by chunk-size.

    3. **Last Chunk**:
      - The last chunk has a chunk-size of zero, optionally followed by chunk extensions, and then CRLF.
      - **Syntax**:
        ```
        last-chunk = 1*("0") [ chunk-ext ] CRLF
        ```

    4. **Trailer Section**:
      - The trailer section is optional and contains additional header fields. It is followed by CRLF.

    #### Example of Chunked Transfer Coding

    Here's a simple example of how a chunked transfer coding response might look:

    ```
    HTTP/1.1 200 OK
    Transfer-Encoding: chunked
    Content-Type: text/plain

    4\r\n
    Wiki\r\n
    5\r\n
    pedia\r\n
    E\r\n
    in
    \r\nchunks.\r\n
    0\r\n
    \r\n
    ```

    - The `4\r\nWiki\r\n` indicates that the first chunk is 4 bytes long, and the data is "Wiki".
    - The `5\r\npedia\r\n` indicates that the second chunk is 5 bytes long, and the data is "pedia".
    - The `E\r\nin\r\nchunks.\r\n` indicates that the third chunk is 14 bytes long, and the data is "in\r\nchunks." (note that E in hexadecimal is 14 in decimal).
    - The `0\r\n\r\n` indicates the last chunk with a size of 0 bytes, followed by an empty line, signaling the end of the chunked message.

  #### Important Points

    1. **Unknown Content Length**:
      - Chunked transfer coding allows the server to start sending the response without knowing the total content length. Each chunk is sent as soon as it is available.

    2. **Connection Persistence**:
      - It helps in maintaining connection persistence, allowing the server to send parts of the response as they are generated, without closing the connection.

    3. **Mandatory Support**:
      - HTTP/1.1 clients and servers must be able to parse and decode chunked transfer coding.

    4. **Handling Large Chunks**:
      - Recipients must be able to handle large chunk sizes properly, avoiding integer conversion overflows or precision loss.

    5. **Parameters**:
      - The chunked transfer coding does not define any parameters. If any parameters are present, they should be treated as errors.

  ## 7.1.1 Chunk Extensions
      Additional metadata or control information to be associated with each chunk of data in a chunked transfer encoding. Randomizing the size of the message body for security or other purposes.

    #### Syntax of Chunk Extensions

      - **Basic Structure**:
        ```
        chunk-ext = *( BWS ";" BWS chunk-ext-name [ BWS "=" BWS chunk-ext-val ] )
        ```
        - `BWS` (Bad White Space) can be optional whitespace.
        - Each chunk extension starts with a semicolon (`;`).
        - `chunk-ext-name` is a token that specifies the name of the extension.
        - `chunk-ext-val` (optional) is either a token or a quoted string that provides the value of the extension.

      - **Components**:
        - **chunk-ext-name**: This is the name of the extension and follows the syntax rules of a token (a sequence of characters).
        - **chunk-ext-val**: This is the value associated with the extension name and can be a token or a quoted string.

    #### Example of Chunked Transfer Coding with Extensions

      Here's an example of chunked transfer coding with chunk extensions:

      ```
      4;signature="abcd1234"\r\n
      Wiki\r\n
      5;hash=xyz\r\n
      pedia\r\n
      0\r\n
      \r\n
      ```

      - The first chunk has a size of 4 bytes and includes a chunk extension `signature="abcd1234"`.
      - The second chunk has a size of 5 bytes and includes a chunk extension `hash=xyz`.
      - The last chunk has a size of 0 bytes, indicating the end of the chunked message.

      #### Handling and Ignoring Unrecognized Extensions

      - **Recipient Requirements**:
        - A recipient (e.g., a client or intermediary) must ignore any chunk extensions that it does not recognize. This ensures that unrecognized metadata does not interfere with the processing of the message body.

      - **Server Limitations**:
        - Servers should limit the total length of chunk extensions in a request to a reasonable amount. This is to prevent abuse or excessive use of resources. If it exceed thelength, server can respond with an appropriate 4xx (Client Error) response code.

      #### Use Cases for Chunk Extensions
        - Chunk extensions are typically used in specialized HTTP services where both the client and server have an agreement on their use, such as in long polling scenarios.

  ## 7.1.2. Chunked Trailer Section
    Allows the sender to include additional metadata fields at the end of the message. These trailer fields can be useful for various purposes such as:

    - **Message Integrity Check**: Including a checksum or hash to verify the integrity of the received message.
    - **Digital Signature**: Providing a digital signature to authenticate the message.
    - **Post-Processing Status**: Indicating the status of the message after it has been processed or generated.

    #### Syntax of the Trailer Section

    The trailer section is defined as:
    ```
    trailer-section = *( field-line CRLF )
    ```

    - **field-line**: This represents a single header field in the HTTP message, consisting of a field name, a colon, and a field value.

    #### Example of a Chunked Trailer Section

    Consider an example where a chunked message includes trailer fields:
    ```
    4\r\n
    Wiki\r\n
    5\r\n
    pedia\r\n
    0\r\n
    Content-MD5: Q2hlY2sgSW50ZWdyaXR5IQ==\r\n
    Signature: d41d8cd98f00b204e9800998ecf8427e\r\n
    \r\n
    ```

    - The message body consists of two chunks ("Wiki" and "pedia").
    - The chunk size of the final chunk is 0, indicating the end of the message body.
    - Following the final chunk size of 0, the trailer section includes two fields: `Content-MD5` and `Signature`.

    #### Handling Trailer Fields by Recipients

    - **Selective Retention or Discard**: Recipients (e.g., clients or intermediaries) may choose to retain or discard the trailer fields after removing the chunked transfer coding.
    - **Storage and Forwarding**: If a recipient retains trailer fields, it has two options:
      - Store or forward the trailer fields separately from the header fields.
      - Merge the trailer fields into the header section.

    - **Merging Trailer Fields**: Merging trailer fields into the header section is not always allowed. It is only permissible if the corresponding header field definitions explicitly state that it is safe.

    ## 7.1.3. Decoding Chunked:
  
      ```
      length := 0
      read chunk-size, chunk-ext (if any), and CRLF
      while (chunk-size > 0) {
        read chunk-data and CRLF
        append chunk-data to content
        length := length + chunk-size
        read chunk-size, chunk-ext (if any), and CRLF
      }
      read trailer field
      while (trailer field is not empty) {
        if (trailer fields are stored/forwarded separately) {
            append trailer field to existing trailer fields
        }
        else if (trailer field is understood and defined as mergeable) {
            merge trailer field with existing header fields
        }
        else {
            discard trailer field
        }
        read trailer field
      }
      Content-Length := length
      Remove "chunked" from Transfer-Encoding

      ```
  ### 7.2 Transfer Codings for Compression

    1. **compress (and x-compress)**
      - **Definition**: Uses the LZW (Lempel-Ziv-Welch) compression algorithm.
      - **Usage**: This is an older compression method and is not widely used in modern HTTP communications.

    2. **deflate**
      - **Definition**: Uses the zlib structure with the deflate compression algorithm.
      - **Usage**: Commonly used for compressing data in a more efficient manner compared to "compress".

    3. **gzip (and x-gzip)**
      - **Definition**: Uses the gzip file format, which includes a CRC (Cyclic Redundancy Check) for error-checking along with the deflate algorithm.
      - **Usage**: Widely used and preferred for its efficiency and widespread support across HTTP clients and servers.

    ### Key Points
    - **No Parameters**: These compression codings do not define any parameters. If present, treated as an error.

    ### Example Usage

    In HTTP headers, the `Transfer-Encoding` field can specify these compression codings. Here is an example of how they might appear in an HTTP response:

    ```http
    HTTP/1.1 200 OK
    Content-Type: text/plain
    Transfer-Encoding: gzip

    <compressed data>
    ```
  ## 7.3. Negotiating Transfer Codings

    The TE (Transfer Encoding) header field in HTTP/1.1 is used for:
    1. Indicate which transfer codings, besides the default `chunked`, it is willing to accept in the response. 
    2. Indicate if the client can handle trailer fields in a chunked transfer coding.

    ### Multiple Transfer Codings
    - When multiple transfer codings are acceptable, The client can rank the acceptable transfer codings using a `q` parameter ranges from 0 (not acceptable) to 1 (most preferred).

    #### Examples

    1. **Single Coding Preference**:
      ```http
      TE: deflate
      ```
      This indicates that the client accepts the `deflate` transfer coding.

    2. **Empty TE Field**:
      ```http
      TE:
      ```
      This indicates that the client only accepts the default `chunked` transfer coding.

    3. **Multiple Codings with Ranking**:
      ```http
      TE: trailers, deflate;q=0.5
      ```
      This indicates that the client accepts trailer fields and `deflate` transfer coding with a preference rank of 0.5.

    #### Connection Header
    - The TE header field applies only to the immediate connection.
    - To ensure that the TE header field is not forwarded by intermediaries that do not support its semantics, the client must also include a `TE` connection option within the `Connection` header field.

      Example:
      ```http
      Connection: TE
      ```

### 8. Handling Incomplete Messages:
  ## Cases:
  1. **For Servers**:
    When a server receives an incomplete request message, such as when a request is canceled or times out, it has the option to send an error response before closing the connection.

  2. **For Clients**:
    When a client receives an incomplete response message, it must treat the message as incomplete. This can happen due to:
    - Premature connection closure.
    - Failure in decoding chunked transfer coding.

  ## Key Scenarios:
    1. **Header Section Incomplete**:
      - If a response terminates in the middle of the header section (before the empty line is received), the client might need to repeat the request to determine the appropriate action.

    2. **Chunked Transfer Coding**:
      - A message using chunked transfer coding is incomplete if the final zero-sized chunk is not received.

    3. **Content-Length**:
      - A message with a valid `Content-Length` is incomplete if received message body is smaller than specified length.

    4. **Neither Chunked Nor Content-Length**:
      - A response without chunked transfer coding or `Content-Length` is considered complete if the connection closes naturally and the header section was received intact. However, if an error occurs at the connection level (e.g., an "incomplete close" in TLS), the response is considered incomplete.

## 9. Connection Management:

  #### Independence from Underlying Protocols:
  - **Transport Independence**:
    - HTTP operates independently of the underlying transport or session-layer protocols.
    - It only requires a reliable transport with in-order delivery of requests and responses.
    - How HTTP maps its structures onto transport protocol data units is beyond its specification.

  #### Determining Connection Protocols:
  - **Client Configuration and URI**:
    - Connection protocols for HTTP interactions are determined by client configuration and the target URI.
    - **Example**: The "http" URI scheme typically implies using TCP over IP with a default TCP port of 80.
    - Clients might be configured to use proxies, different ports, or other protocols.

  #### Connection Management in HTTP Implementations:
  - **Connection State**:
    - Managing current connections.
    - Establishing new connections or reusing existing ones.
    - Processing received messages.
    - Detecting connection failures.
    - Closing connections when needed.

  - **Parallel Connections**:
    - Clients often maintain multiple parallel connections, sometimes more than one per server endpoint.
    - Servers are designed to handle thousands of concurrent connections and manage request queues to ensure fair use and detect potential denial-of-service attacks.

  ## 9.1 Establishment:
    - HTTP defines how messages are formatted and exchanged, the details of establishing and managing the underlying transport connections fall within the domain of transport protocols like TCP, UDP, and TLS. 

  ## 9.2. Associating a Response to a Request:
    HTTP/1.1 defines rules for associating response messages with their corresponding request messages on a connection:

    1. **Order of Response Arrival**: 
      - HTTP/1.1 does not assign a unique identifier to each request, which means it relies on the order of response arrival to match each response to its corresponding request.
      - Responses must arrive in the same order that requests were made on the same connection, except in cases where informational responses (1xx) may precede the final response.

    2. **Handling Multiple Outstanding Requests**:
      - If a client has multiple outstanding requests (requests that have been sent but have not yet received a final response), it must maintain a list of these requests in the order they were sent.
      - Each received response must be associated with the first outstanding request that has not yet received a final (non-1xx) response.
      - This ensures that responses are processed in the correct order relative to their corresponding requests.

    3. **Handling Unexpected Data**:
      - If data is received on a connection where there are no outstanding requests (e.g., after all requests have received final responses or due to an unexpected transmission), the client should not treat this data as a valid response.
      - The client is recommended to close the connection in such cases, as the boundaries between messages become ambiguous. However, if the received data consists only of one or more CRLF , it can be discarded.

  ## 9.3. Persistent:
    HTTP/1.1 defaults to using persistent connections unless specified otherwise:
    - **Connection Header**: `Connection: close` indicates the connection won't persist after the current response.
    - **HTTP/1.0 Compatibility**: In HTTP/1.0, a `Connection: keep-alive` header indicates the connection should persist unless explicitly closed.
    - A server that does not support persistent connections MUST send the "close" connection option in every response message that does not have a 1xx (Informational) status code.    
    
   - For a connection to remain persistent in HTTP/1.1, each message (request or response) must have a self-defined message length (length explicitly stated in the headers).
   - If the server sends a response, it must either read the entire request message body or close the connection afterward. This prevents any remaining data on the connection from being mistaken as part of the next request.

   - Clients must ensure they read the entire response message body if they intend to reuse the connection for subsequent requests. This ensures that the connection remains in a consistent state for further communication.

   - HTTP/1.0 clients often used the `Keep-Alive` header to request persistent connections. However, due to problems and inconsistencies with how `Keep-Alive` was implemented in various HTTP/1.0 clients, proxy servers must not maintain persistent connections with HTTP/1.0 clients. This restriction helps avoid compatibility and reliability issues.

    ## 9.3.1 Retrying Requests
    - Clients can retry requests under certain conditions, typically after network failures or asynchronous closures.
    - **Idempotent Methods**: Safe methods (like GET, HEAD, OPTIONS) are idempotent and suitable for automatic retries and pipelining.
    - **Non-Idempotent Methods**: Methods like POST should not be pipelined until the final response for the previous request is received.

    ### 9.3.2 Pipelining
    - **Request Pipelining**: Clients can send multiple requests without waiting for each response.
    - **Server Handling**: Servers should process pipelined requests in parallel and respond in the order received.
    - **Retry Mechanism**: Clients should retry unanswered pipelined requests if the connection closes prematurely, especially for idempotent methods. However, it should not retry immediately to prevents potentially lost or misinterpreted responses due to a prematurely closed connection.

    - **Intermediaries**: Can forward pipelined requests and handle retries if necessary, ensuring responses are forwarded in order.

  ## 9.4. Concurrency

    1. **Limit on Simultaneous Connections**:
      - There is no specific mandated limit on the number of simultaneous connections a client can maintain to a server.
      - Clients are encouraged to be conservative when opening multiple connections to a server to avoid resource exhaustion.

    2. **Head-of-Line Blocking**:
      - Multiple connections can help avoid the "head-of-line blocking" issue, where a single slow request can delay subsequent requests on the same connection. However, each additional connection consumes server resources.

    3. **Network Congestion**:
      - Using many connections can lead to undesirable side effects, especially in congested networks.
      - In uncongested networks, a large number of connections can still cause congestion due to their aggregate and synchronized sending behavior.

    4. **Server Rejection of Excessive Connections**:
      - Servers might reject traffic that appears abusive, such as an excessive number of open connections from a single client, to protect against denial-of-service attacks.

  ## 9.5. Failures and Timeouts:

    1. **Timeout Values**:
      - Servers typically have a timeout value for inactive connections, which may vary.
      - Proxy servers might have longer timeout values as clients are expected to make more connections.
      - The HTTP/1.1 specification does not mandate a specific timeout duration for either clients or servers because its presistent.

    2. **Graceful Close**:
      - Clients and servers that wish to close an idle connection should do so gracefully.
      - Implementations should monitor open connections for closure signals and respond promptly to release system resources.

    3. **Connection Closure**:
      - Any party (client, server, or proxy) can close the transport connection at any time.
      - This can lead to situations where a client may start a new request while the server decides to close what it considers an idle connection.

    4. **Sustaining Persistent Connections**:
      - Servers should sustain persistent connections when possible and rely on the underlying transport's flow-control mechanisms (mechanism of TCP that change rates of data transmission depending on the traffic) to handle temporary overloads.
      - Terminating connections in expectation of client retries can worsen network congestion and server load so it is better to depend on flow-control mechanisms.

    5. **Monitoring Network Connections**:
      - A client sending a message body should monitor for error responses while transmitting.
      - If a client receives an indication that the server is closing the connection and does not want to receive the message body, the client should immediately stop transmitting and close the connection.
