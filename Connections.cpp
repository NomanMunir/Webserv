#include "Connections.hpp"
#include "response/Response.hpp"
#include "request/Request.hpp"


void Connections::setServer(int fd)
{
    setNonBlocking(fd);
    EV_SET(&changeList[0], fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);

    if (kevent(kqueueFd, changeList, 1, NULL, 0, NULL) == -1)
    {
        std::cerr << "Error: " << strerror(errno) << std::endl;
        close(fd);
        throw std::exception();
    }
}

Connections::Connections(std::vector<int> fds, Parser &configFile) : serverSockets(fds), configFile(configFile)
{
    this->kqueueFd = kqueue();
    if (this->kqueueFd == -1)
    {
        std::cerr << "Error: " << strerror(errno) << std::endl;
        throw std::exception();
    }

    for (size_t i = 0; i < serverSockets.size(); i++)
        setServer(serverSockets[i]);
}

Connections::Connections(const Connections &c) : serverSockets(c.serverSockets), kqueueFd(c.kqueueFd), configFile(c.configFile)
{
    for (size_t i = 0; i < serverSockets.size(); i++)
        setServer(serverSockets[i]);
}

Connections &Connections::operator=(const Connections &c)
{
    if (this == &c)
        return *this;
    serverSockets = c.serverSockets;
    kqueueFd = c.kqueueFd;
    configFile = c.configFile;
    for (size_t i = 0; i < serverSockets.size(); i++) 
        setServer(serverSockets[i]);
    return *this;
}

Connections::~Connections()
{
    for (size_t i = 0; i < serverSockets.size(); i++) 
        close(serverSockets[i]);
    close(kqueueFd);
}

void Connections::setNonBlocking(int fd) 
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) 
    {
        perror("fcntl");
        exit(EXIT_FAILURE);
    }
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl");
        exit(EXIT_FAILURE);
    }
}

void Connections::setClient(int fd) 
{
    setNonBlocking(fd);
    EV_SET(&changeList[0], fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
    EV_SET(&changeList[1], fd, EVFILT_TIMER, EV_ADD | EV_ENABLE, 0, 3000000, NULL);

    if (kevent(kqueueFd, changeList, 2, NULL, 0, NULL) == -1)
    {
        std::cerr << "Error: " << strerror(errno) << std::endl;
        close(fd);
        throw std::exception();
    }
    clients[fd] = Client(fd);
}

void Connections::removeClient(int fd) 
{
    EV_SET(&changeList[0], fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
    EV_SET(&changeList[1], fd, EVFILT_TIMER, EV_DELETE, 0, 0, NULL);

    if (kevent(kqueueFd, changeList, 2, NULL, 0, NULL) == -1)
        std::cerr << "Error: " << strerror(errno) << std::endl;
    clients.erase(fd);
    close(fd);
    std::cout << "Connection closed by client on socket fd " << fd << std::endl;
}

bool Connections::addClient(int serverSocket) {
    struct sockaddr_in clientAddr;
    socklen_t clientLen = sizeof(clientAddr);
    int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientLen);
    if (clientSocket < 0)
    {
        std::cerr << "Error accepting client connection" << std::endl;
        return false;
    }
    std::cout << "New connection, socket fd is " << clientSocket << ", ip is : " << inet_ntoa(clientAddr.sin_addr) << ", port : " << ntohs(clientAddr.sin_port) << std::endl;
    setClient(clientSocket);
    // setTimeout(clientSocket);
    return true;
}

void Connections::loop() 
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
            if (events[i].flags & EV_ERROR) 
            {
                std::cerr << "EV_ERROR: " << strerror(events[i].data) << std::endl;
                continue;
            }

            if (events[i].filter == EVFILT_TIMER)
                handleTimeoutEvent(events[i].ident);
            else if (events[i].filter == EVFILT_READ)
            {
                if (std::find(serverSockets.begin(), serverSockets.end(), events[i].ident) != serverSockets.end()) 
                {
                    if(!addClient(events[i].ident))
                        continue;
                }
                else
                {
                    std::cout << "Read event on client: " << events[i].ident << std::endl;
                    handleReadEvent(events[i].ident);
                }
            }
            else if (events[i].filter == EVFILT_WRITE)
            {
                std::cout << "Write event on client: " << events[i].ident << std::endl;
                handleWriteEvent(events[i].ident);
            }
        }
    }
}

bool Connections::handleChunkedData(Client &client)
{
    std::string &buffer = client.getReadBuffer();
    std::string &body = client.getRequest().getBody().getContent();
    // std::cout << "chunked data: " << buffer << std::endl;
    std::string::size_type pos = 0;
    while (true) {
        std::string::size_type chunkSizeEnd = buffer.find("\r\n", pos);
        if (chunkSizeEnd == std::string::npos) {
            return false;
        }

        std::string chunkSizeHex = buffer.substr(pos, chunkSizeEnd - pos);
        size_t chunkSize;
        std::istringstream(chunkSizeHex) >> std::hex >> chunkSize;
        pos = chunkSizeEnd + 2;

        if (chunkSize == 0) {
            buffer.erase(0, pos + 2);
            client.getRequest().setComplete(true);
            return true;
        }

        if (buffer.size() < pos + chunkSize + 2) {
            return false;
        }

        body.append(buffer, pos, chunkSize);
        pos += chunkSize + 2;
    }
}

void Connections::handleReadEvent(int clientFd)
{
    Client &client = clients.at(clientFd);
    Headers& header = client.getRequest().getHeaders();

    if (!ft_recv_header(clientFd, header.getRawHeaders()))
        return (removeClient(clientFd));
    header.parseHeader(client.getResponse());

    if (client.getRequest().getHeaders().isComplete())
    {
        if (client.getRequest().isBodyExist(configFile, client.getResponse()))
        {
            if (client.getRequest().isChunked())
            {
                if (!handleChunkedData(client))
                    return;
            }
            else
            {
                int contentLength = atoi(client.getRequest().getHeaders().getValue("Content-Length").c_str());
                if (client.getRequest().getBody().readBody(clientFd, contentLength))
                    return (removeClient(clientFd));
                client.getRequest().getBody().printBody();
            }
        }
        client.getRequest().handleRequest(this->configFile, client.getResponse());
        if (client.getRequest().isComplete())
        {
            client.getResponse().handleResponse(client.getRequest());
            client.getWriteBuffer() = client.getResponse().getResponse();
            client.setWritePending(true);
            client.setReadPending(false);
            client.getReadBuffer().clear();
            this->setWriteEvent(clientFd);
        }
    }
}

void Connections::handleWriteEvent(int clientFd) {
    Client &client = clients.at(clientFd);
    if (!client.isWritePending()) return;

    ssize_t bytesWritten = send(clientFd, client.getWriteBuffer().c_str(), client.getWriteBuffer().size(), 0);
    if (bytesWritten < 0) {
        perror("send");
        removeClient(clientFd);
        return;
    }

    // Remove sent data from the buffer
    client.getWriteBuffer().erase(0, bytesWritten);
    if (client.getWriteBuffer().empty()) 
    {
        client.setWritePending(false);
        // Remove write event if no more data to send
        removeWriteEvent(clientFd);

        if (!client.isKeepAlive()) 
        {
            std::cout<<"Closed bc of IsKeepAlive " << clientFd << std::endl;
            removeClient(clientFd);
            return;
        }

        EV_SET(&changeList[1], clientFd, EVFILT_TIMER, EV_ADD | EV_ENABLE, 0, 3000000, NULL);
        if (kevent(kqueueFd, changeList, 1, NULL, 0, NULL) == -1)
        {
            std::cerr << "Error resetting keep-alive timer: " << strerror(errno) << std::endl;
            removeClient(clientFd);
        }
        else
            client.reset();
    }
}

void Connections::handleTimeoutEvent(int clientFd) 
{
    std::cout << "Timeout event on client: " << clientFd << std::endl;
    removeClient(clientFd);
}


void Connections::removeWriteEvent(int clientFd) {
    struct kevent evSet;
    EV_SET(&evSet, clientFd, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);

    if (kevent(kqueueFd, &evSet, 1, NULL, 0, NULL) == -1) {
        std::cerr << "Error removing write event: " << strerror(errno) << std::endl;
        close(clientFd);
        clients.erase(clientFd);
    }
}

void Connections::setWriteEvent(int clientFd) 
{
    struct kevent evSet;
    EV_SET(&evSet, clientFd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
    
    if (kevent(this->kqueueFd, &evSet, 1, NULL, 0, NULL) == -1) 
    {
        std::cerr << "Error setting write event: " << strerror(errno) << std::endl;
        removeClient(clientFd);  // Handle error by removing client
    }
}

void Connections::setTimeout(int fd)
{
    EV_SET(&changeList[0], fd, EVFILT_TIMER, EV_ADD | EV_ENABLE, 0, 0, NULL); // 5 seconds timeout
    if (kevent(kqueueFd, changeList, 1, NULL, 0, NULL) == -1)
    {
        std::cerr << "Error: " << strerror(errno) << std::endl;
        close(fd);
        throw std::exception();
    }
}

// Connections::~Connections() { }

// Connections::Connections(std::vector<std::string> fds) : serverSocket(fds)
// {
// 	this->kqueueFd = kqueue();
// 	if (this->kqueueFd == -1)
// 	{
// 		std::cerr << "Error: " << strerror(errno) << std::endl;
// 		throw std::exception();
// 	}
// 	setClient(fds);
// }

// void setNonBlocking(int fd) {
//     int flags = fcntl(fd, F_GETFL, 0);
//     if (flags == -1) {
//         perror("fcntl");
//         exit(EXIT_FAILURE);
//     }
//     if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
//         perror("fcntl");
//         exit(EXIT_FAILURE);
//     }
// }

// void Connections::removeClient(int fd)
// {
// 	EV_SET(&this->change, fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
//     std::cout <<"Connection closed by client on socket fd " << fd << std::endl;

// 	if (kevent(this->kqueueFd, &this->change, 1, NULL, 0, NULL) == -1)
// 	{
// 		std::cerr << "Error: " << strerror(errno) << std::endl;
//         close(fd);
// 		throw std::exception();
// 	}
// 	close(fd);
// }
// void Connections::setClient(int fd)
// {
// 	// EV_SET(&this->change, fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
// 	setNonBlocking(fd);
//     EV_SET(&change, fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
// 	if (kevent(this->kqueueFd, &this->change, 1, NULL, 0, NULL) == -1)
// 	{
// 		std::cerr << "Error: " << strerror(errno) << std::endl;
//         close(fd);
// 		throw std::exception();
// 	}
// }

// bool Connections::peekRequest(int clientSocket)
// {
//     char buffer[1];
//     ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer), MSG_PEEK);
//     if (bytesRead < 0)
//     {
//         perror("Error reading from client socket");
//         return true;
//     }
//     else if (bytesRead == 0)
//     {
//         return true;
//     }
//     return false;
// }

// bool Connections::handleClient(int clientSocket, Parser &configFile)
// {
//     try
//     {
//         if (peekRequest(clientSocket))
//         {
//             removeClient(clientSocket);
//             return (false);
//         }
//         Response response(clientSocket);
//         Request request(clientSocket, configFile, response);

//         // Handle the response and send it to the client
//         response.handleResponse(request);
//         std::string clientResponse = response.getResponse();
//         if (!responseClient(clientSocket, clientResponse))
//             removeClient(clientSocket);
//         if (response.getIsConClosed())
//         {
//             removeClient(clientSocket);
//             return (false);
//         }

//         std::string keepAlive = request.getHeaders().getValue("Connection");
//         if (keepAlive != "keep-alive")
//             removeClient(clientSocket);
//     }
//     catch (const std::exception &e)
//     {
//         std::cerr << "Error handling request: " << e.what() << std::endl;
//         removeClient(clientSocket);
//         return (false);
//     }
//     return (true);
// }

// bool Connections::addClient()
// {
//     struct sockaddr_in clientAddr;
//     socklen_t clientLen = sizeof(clientAddr);
//     int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientLen);
//     if (clientSocket < 0)
//     {
//         std::cerr << "Error accepting client connection" << std::endl;
//         return false;
//     }
//     std::cout << "New connection, socket fd is " << clientSocket << ", ip is : " << inet_ntoa(clientAddr.sin_addr) << ", port : " << ntohs(clientAddr.sin_port) << std::endl;
//     setClient(clientSocket);
//     return true;
// }

// void Connections::loop(Parser &configFile)
// {
// 	while (true)
// 	{
// 		int nev = kevent(this->kqueueFd, NULL, 0, this->events, MAX_EVENTS, NULL);
// 		if (nev == -1)
// 		{
// 			std::cerr << "Error: " << strerror(errno) << std::endl;
// 			throw std::exception();
// 		}
// 		for (int i = 0; i < nev; i++)
// 		{
// 			if (this->events[i].ident == this->serverSocket)
// 			{
// 				if(!this->addClient())
// 					continue;
// 			}
// 			else if (this->events[i].filter == EVFILT_READ)
// 			{
// 				if (!this->handleClient(this->events[i].ident, configFile))
// 					continue;
// 			}
// 		}
// 	}
// }
