# Config File

# Server block 1
server {
    # Choose the port and host
    listen 80;
    server_name example.com www.example.com;
    
    # Default error pages
    error_page 400 /errors/400.html;
    error_page 403 /errors/403.html;
    error_page 404 /errors/404.html;
    error_page 413 /errors/413.html;
    error_page 414 /errors/414.html;
    error_page 500 /errors/500.html;
    error_page 501 /errors/501.html;

    # Limit client body size
    client_max_body_size 1m;

    # Routes configuration
    location / {
        root /var/www/html;
        index index.html;
    }

    location /images/ {
        root /var/www/html;
        autoindex on;
    }

    location /upload/ {
        root /var/www/html;
        methods POST;
        upload_store /var/www/uploads;
    }

    location /redirect {
        return 301 http://example.org/new_location;
    }

    location /cgi-bin/ {
        root /var/www/cgi-bin;
        cgi_extension .cgi;
        cgi_handler /usr/bin/perl;
    }
}

# Server block 2
server {
    # Choose the port and host
    listen 8080;
    server_name test.com www.test.com;

    # Default error pages
    error_page 404 /errors/404.html;
    error_page 403 /errors/403.html;

    # Limit client body size
    client_max_body_size 500k;

    # Routes configuration
    location / {
        root /srv/www;
        index index.html;
    }

    location /private/ {
        root /srv/www;
        methods GET POST;
        autoindex off;
    }

    location /api/ {
        root /srv/api;
        cgi_extension .py;
        cgi_handler /usr/bin/python3;
    }
}


1. Choose the port and host of each ’server’.
2. Setup the server_names or not.
3. The first server for a host:port will be the default for this host:port (that means
it will answer to all the requests that don’t belong to an other server).
4. Setup default error pages.
5. Limiting client body size refers to restricting the maximum amount of data that a client can send in the body of an HTTP request
6. • Setup routes with one or multiple of the following rules/configuration (routes wont
be using regexp):
◦ Define a list of accepted HTTP methods for the route.
◦ Define a HTTP redirection.
◦ Define a directory or a file from where the file should be searched (for example,
if url /kapouet is rooted to /tmp/www, url /kapouet/pouic/toto/pouet is
/tmp/www/pouic/toto/pouet).
◦ Turn on or off directory listing.
7
Webserv This is when you finally understand why a URL starts with HTTP
◦ Set a default file to answer if the request is a directory.
◦ Execute CGI based on certain file extension (for example .php).
◦ Make it work with POST and GET methods.
◦ Make the route able to accept uploaded files and configure where they should
be saved.
∗ Do you wonder what a CGI is?
∗ Because you won’t call the CGI directly, use the full path as PATH_INFO.
∗ Just remember that, for chunked request, your server needs to unchunk
it, the CGI will expect EOF as end of the body.
∗ Same things for the output of the CGI. If no content_length is returned
from the CGI, EOF will mark the end of the returned data.
∗ Your program should call the CGI with the file requested as first argument.
∗ The CGI should be run in the correct directory for relative path file access.
∗ Your server should work with one CGI (php-CGI, Python, and so forth).