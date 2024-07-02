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
