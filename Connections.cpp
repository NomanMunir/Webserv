/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connections.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmunir <nmunir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/20 15:49:37 by nmunir            #+#    #+#             */
/*   Updated: 2024/07/20 17:44:55 by nmunir           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Connections.hpp"
#include "response/Response.hpp"
#include "request/Request.hpp"

Connections::~Connections() { }

static void setNonBlocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

void Connections::removeClient(int fd)
{
	EV_SET(&this->change, fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
	if (kevent(this->kqueueFd, &this->change, 1, NULL, 0, NULL) == -1)
	{
		std::cerr << "Error: " << strerror(errno) << std::endl;
		throw std::exception();
	}
	close(fd);
}
void Connections::setClient(int fd)
{
	EV_SET(&this->change, fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
	if (kevent(this->kqueueFd, &this->change, 1, NULL, 0, NULL) == -1)
	{
		std::cerr << "Error: " << strerror(errno) << std::endl;
		throw std::exception();
	}
}

bool Connections::peekRequest(int clientSocket)
{
    char buffer[1024];

    ssize_t bytesRead = recv(clientSocket, buffer, 1, MSG_PEEK);
    if (bytesRead < 0)
    {
        perror("Error reading from client socket");
        return (true);
    }
    else if (bytesRead == 0)
    {
        std::cout <<"Connection closed by client" << std::endl;
        return (true);
    }
    else
    {
        buffer[bytesRead] = '\0';
        if (strlen(buffer) == 0)
        {
            std::cout << "Connection closed by client" << std::endl;
            return (true);
        }
    }
    return false;
}

bool Connections::handleClient(int clientSocket, Parser &configFile)
{
    try
    {
        if (peekRequest(clientSocket))
        {
            removeClient(clientSocket);
            return (false);
        }
        Response response(clientSocket);
        Request request(clientSocket, configFile, response);

        // Handle the response and send it to the client
        response.handleResponse(request);
        std::string clientResponse = response.getResponse();
        
        if (!responseClient(clientSocket, clientResponse))
            removeClient(clientSocket);

        std::string keepAlive = request.getHeaders().getValue("Connection");
        if (keepAlive != "keep-alive")
        {
            EV_SET(&change, clientSocket, EVFILT_READ, EV_DELETE, 0, 0, NULL);
            if (kevent(kqueueFd, &change, 1, NULL, 0, NULL) == -1)
                perror("Error removing client socket from kqueue");
            close(clientSocket);
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error handling request: " << e.what() << std::endl;
        EV_SET(&change, clientSocket, EVFILT_READ, EV_DELETE, 0, 0, NULL);
        if (kevent(kqueueFd, &change, 1, NULL, 0, NULL) == -1)
            perror("Error removing client socket from kqueue");
        close(clientSocket);
        return (false);
    }
    return (true);
}

Connections::Connections(int fd) : serverSocket(fd)
{
	this->kqueueFd = kqueue();
	if (this->kqueueFd == -1)
	{
		std::cerr << "Error: " << strerror(errno) << std::endl;
		throw std::exception();
	}
	setNonBlocking(fd);
	EV_SET(&change, fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
	if (kevent(this->kqueueFd, &this->change, 1, NULL, 0, NULL) == -1)
	{
		std::cerr << "Error: " << strerror(errno) << std::endl;
		throw std::exception();
	}
}

bool Connections::addClient()
{
    struct sockaddr_in clientAddr;
    socklen_t clientLen = sizeof(clientAddr);
    int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientLen);
    if (clientSocket < 0)
    {
        std::cerr << "Error accepting client connection" << std::endl;
        return false;
    }
    std::cout << "New connection, socket fd is " << clientSocket << ", ip is : " << inet_ntoa(clientAddr.sin_addr) << ", port : " << ntohs(clientAddr.sin_port) << std::endl;
	setNonBlocking(clientSocket);
    EV_SET(&change, clientSocket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
    if (kevent(kqueueFd, &change, 1, NULL, 0, NULL) == -1)
    {
        perror("Error adding client socket to kqueue");
        close(clientSocket);
        return false;
    }
    return true;
}

void Connections::loop(Parser &configFile)
{
	while (true)
	{
		int nev = kevent(this->kqueueFd, NULL, 0, this->events, MAX_EVENTS, NULL);
		if (nev == -1)
		{
			std::cerr << "Error: " << strerror(errno) << std::endl;
			throw std::exception();
		}
		for (int i = 0; i < nev; i++)
		{
			if (this->events[i].ident == this->serverSocket)
			{
				if(!this->addClient())
					continue;
			}
			else if (this->events[i].filter == EVFILT_READ)
			{
				if (!this->handleClient(this->events[i].ident, configFile))
					continue;
			}
		}
	}
}
