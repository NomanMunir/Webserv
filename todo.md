[*] check for syntex errors
[*] if location block has curly brackets then check for semicolon at the end.
[*] check for missing semicolon at the end of the line
[*] check for invalid directives.
[*] Handle multiple keys for single value like 500 502 /error.html
[*] Handle multiple values for single key like server_names hi there
[*] check files path and directories if they exist.
[*] check repeated keys.
[*] listen can be only port number or with host ip:port ex : listen 127.0.0.1:8080;
[*] If we have the same error code like: 400 400 /error.html 500 400 /500.html do we replace the old 400 value or keep the old one?

## Request:
[ ] Pares http version 1.0

## header
[*] A client MUST send a Host header field in all HTTP/1.1 request messages.

[*] If the target URI includes an authority component, then a client MUST send a field value for Host that is identical to that authority component, excluding any userinfo subcomponent and its "@" delimiter.

[ ] If the authority component is missing or undefined for the target URI, then a client MUST send a Host header field with an empty field value.

[ ] A server MUST respond with a 400 (Bad Request) status code to any HTTP/1.1 request message that lacks a Host header field and to any request message that contains more than one Host header field line or a Host header field with an invalid field value.



[*] link config file with all the classes like client_body_size to how much we are reading; 
[*] split the server names and set check each one.
[*] set default server if we could not find the server_name.
[*] is method allowed in the route block.
[*] fix if there is no location block.
[*] duplicate location in the server block is invalid.
[*] auto indexing for directories.
[*] server directives are repeated
[*] check if the directory is ending with / in nginx. directory ending with / is valid in nginx.
[*] servers with the same server name. How to hanlde them check in nginx?
	Answer: it will use the default one if it finds multiple servers with the same name and same port. Firt it will check the port and then the server name.
[*] server will still run if there is no listen directive.
[*] if there is no root directive in the server block it use it's default folder to serve the files.
[*] if there in no servername it will still run.
[*] it needs a server does not metter even if it's empty. it will still run.
[*] if the location key is "hi" not "/hi" what to do
[*] if root has slash at the end
[*] substring creating problems.
[*] check for file types and set appropriate values.
[*] check Accept header before sending response.



[ ] if there are more server in the config file. we need to set them up for listening.

[ ] Post and delete request;

[ ] genrate a proper response with proper response headers for the request

[ ] chunked encoding.

[*] redirect 
[*] full path in redirect
[*] message "hello world"
[*] only error code
[*] 404 close connection.
