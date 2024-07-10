#include "Server.hpp"
#include <cstring>
#include <fstream>
#include <iostream>
#include <netinet/in.h>
#include <sstream>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

Server::Server()
{
	initSocket();
}

void Server::initSocket()
{
	int	e;

	serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket < 0)
	{
		perror("Error opening socket");
		exit(1);
	}
	e = 1;
	if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &e, sizeof(e)) < 0)
	{
		perror("Error setsockopt option");
		close(serverSocket);
		exit(1);
	}
	if (fcntl(serverSocket, F_SETFL, O_NONBLOCK) < 0)
	{
		perror("Error non-Blocking server");
		close(serverSocket);
		exit(1);
	}
	bzero((char *)&serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_port = htons(8001);
	if (bind(serverSocket, (struct sockaddr *)&serverAddr,
			sizeof(serverAddr)) < 0)
	{
		perror("Error binding socket");
		close(serverSocket);
		exit(1);
	}
	if (listen(serverSocket, SOMAXCONN))
	{
		perror("Error: listen");
		close(serverSocket);
		exit(1);
	}
}

void Server::run()
{
	handleConnections();
}

void Server::handleConnections()
{
	pollfd		event;
	int			client_fd;

	pollManager.addSocket(serverSocket, POLLIN);
		// Add server socket to poll list
	std::cout << "Server socket added to poll list. Waiting for connections..." << std::endl;
	while (true)
	{
		int pollCount = pollManager.waitForEvents();
			// Wait indefinitely for events
		if (pollCount < 0)
		{
			perror("Error: Poll failed");
			break ;
		}
		if (pollCount == 0)
		{
			perror("Error: No event");
			break ;
		}
		for (size_t i = 0; i < pollManager.getSocketsCount(); ++i)
		{
			event = pollManager.getEvent(i);
			if (event.revents & POLLIN)
			{
				// creat new function to handle the new connection
				if (event.fd == serverSocket)
				{
					exeptNewConnection();
				}
                else
                {
                    client_fd = event.fd;
					handleRequest(client_fd);
					std::cout << "iteration" << i<< std::endl;
                	close(client_fd);
					pollManager.removeSocket(i); // Remove from poll list
					--i;      
                    std::cout << "iteration" << i<< std::endl;
                }
                // if(event.revents & POLLOUT)
				// {
				// 	 prepareAndSendResponse(client_fd);

				// 	// Close client connection after handling request                  
				// 		// Adjust index to account for erased element
				// }
                    
                // if(event.revents & POLLIN)
                // {

                // }
                // else
                // {
                //     std::cout << "event.fd: " << event.fd << std::endl;
                //     std::cout << "Error: Unknown event" << std::endl;
                // }
			}
		}
	}
	// Cleanup: Close server socket
	close(serverSocket);
}

// void Server::prepareAndSendResponse(int client_fd) {
//     // Example function to prepare and send a response
//     std::string response = "HTTP/1.1 200 OK\r\nContent-Length: 12\r\n\r\nHello World!";
//     send(client_fd, response.c_str(), response.size(), 0);
// }

void Server::printHeaders()
{
	std::cout << "Headers:\n";
	for (std::map<std::string,
		std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it)
	{
		std::cout << it->first << ": " << it->second << std::endl;
	}
}

void Server::exeptNewConnection()
{
    socklen_t	clientLen;
	int			client_fd;
    struct sockaddr_in clientAddr;

    std::cout << "New connection" << std::endl;
    
    clientLen = sizeof(clientAddr);
	client_fd = accept(serverSocket,(struct sockaddr *)&clientAddr, &clientLen);
	if (client_fd < 0)
    {
        perror("Error accepting client connection");
    }
    if (fcntl(client_fd, F_SETFL, O_NONBLOCK) < 0)
	{
		perror("Error non-Blocking server");
        close(client_fd);
	}
    // Add client socket to poll list
    pollManager.addSocket(client_fd, POLLIN);
}

void Server::handleRequest(int clientSocket)
{
	char	buffer[1024];

	bzero(buffer, 1024);
	read(clientSocket, buffer, 1023);
	// parseFirstLine(buffer);
	storeHeaders(buffer);
	printHeaders();
	std::string response = generateHttpResponse(headers["uri"]);
	sendResponse(clientSocket, response);
}

std::string Server::generateHttpResponse(const std::string &filepath)
{
	std::ifstream file(std::string("." + filepath).c_str());
	if (!file.is_open())
	{
		std::cerr << "Could not open the file: " << filepath << std::endl;
		return ("HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\nFile not found");
	}
	std::stringstream buffer;
	buffer << file.rdbuf();
	file.close();
	std::string content = buffer.str();
	std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: "
		+ std::to_string(content.size()) + "\r\n\r\n" + content;
	return (response);
}

void Server::sendResponse(int clientSocket, const std::string &response)
{
	write(clientSocket, response.c_str(), response.length());
}

void Server::storeHeaders(const std::string &request)
{
	size_t	colonPos;

	std::stringstream requestLine(request.c_str());
	// bool headersDone = false;
	std::getline(requestLine, startLine);
	// parseStartLine(startLine);
	storeFirstLine(startLine);
	std::string header;
	while (std::getline(requestLine, header))
	{
		colonPos = header.find(':');
		if (colonPos != std::string::npos)
		{
			std::string key = header.substr(0, colonPos);
			std::string value = header.substr(colonPos + 1);
			headers[key] = value;
		}
	}
}

void Server::storeFirstLine(const std::string &request)
{
	std::istringstream requestStream(request);
	std::string method;
	std::string uri;
	std::string version;

	requestStream >> method >> uri >> version;
	headers["method"] = method;
	headers["version"] = version;
	headers["uri"] = uri;
}