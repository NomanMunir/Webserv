# Webserv

Webserv is a project developed as part of the 42 Abu Dhabi curriculum, aimed at building a fully functioning HTTP web server from scratch using C++. This project provides a deep understanding of how web servers work, including HTTP request handling, static file serving, and dynamic content generation. The server is designed to be non-blocking, resilient, and compliant with HTTP/1.1 standards.

## Features

- **HTTP Request Handling**: Supports common HTTP methods such as GET, POST, and DELETE.
- **Static File Serving**: Capable of serving static files like HTML, CSS, images, and JavaScript files.
- **Dynamic Content Generation**: Supports CGI execution, allowing dynamic content generation using scripts (e.g., PHP, Python).
- **Configuration File Parsing**: The server is configured via a configuration file inspired by NGINX's configuration format, allowing for detailed customization of server behavior.
- **Non-Blocking I/O**: Utilizes non-blocking I/O with `poll()` for all client-server communication, ensuring high performance and responsiveness.
- **Multi-Port Listening**: Can listen on multiple ports simultaneously, making it suitable for complex server setups.
- **Error Handling**: Provides accurate HTTP response status codes and default error pages.
- **File Uploads**: Supports file uploads via HTTP POST requests.
- **Session Management**: Supports cookies and session management.
- **Stress Testing**: The server is stress-tested to ensure it remains available under high load.

## Requirements

- The server must take a configuration file as an argument or use a default path.
- It must be non-blocking and use only one `poll()` (or equivalent) for all I/O operations between the client and the server, including listening.
- The server must support multiple clients and handle requests without blocking.
- The server must implement accurate HTTP response status codes and provide default error pages if none are specified.
- The server must support at least GET, POST, and DELETE methods.
- The server must handle multiple CGI scripts and allow for file uploads.
- Configuration files should allow for:
  - Port and host selection
  - Server name setup
  - Default error pages
  - Client body size limitation
  - HTTP method restrictions for routes
  - Directory listing configuration
  - CGI execution based on file extensions

## Configuration File

The configuration file allows for detailed customization of the server, including:

- **Port and Host Selection**: Specify the port and host for each server.
- **Server Names**: Set up server names or use default names.
- **Default Error Pages**: Define custom error pages for different HTTP status codes.
- **Client Body Size Limitation**: Limit the size of the client's body to prevent resource exhaustion.
- **Route Configuration**: Set up routes with specific rules such as HTTP method restrictions, redirections, directory listing, and CGI execution.
- **File Uploads**: Configure routes to accept file uploads and specify where the files should be stored.

## Learning Outcomes

During the development of Webserv, the following key concepts and skills were learned:

- **File Descriptor (fd) Queues**: Managing multiple client connections simultaneously using non-blocking file descriptors.
- **HTTP Protocols**: Implementing and understanding the nuances of HTTP/1.1, including handling of GET, POST, and DELETE methods.
- **Configuration Management**: Parsing and applying configuration settings from a file, similar to how NGINX handles its configurations.
- **CGI Handling**: Implementing CGI execution for dynamic content generation, including handling chunked requests and EOF marking.
- **Stress Testing**: Ensuring the server remains resilient and available under high traffic and load conditions.

## Usage

To run the Webserv server:

1. Compile the server using the provided Makefile.
   ```bash
   make
