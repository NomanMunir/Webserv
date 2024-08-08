#include "Connections.hpp"
#include "response/Response.hpp"
#include "request/Request.hpp"

#define SET_TIMEOUT 300000

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

Connections::Connections(std::vector<int> fds, Parser &configFile)
    : serverSockets(fds),
        configFile(configFile)
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

Connections::Connections(const Connections &c)
    : serverSockets(c.serverSockets),
        kqueueFd(c.kqueueFd),
        configFile(c.configFile)
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
    EV_SET(&changeList[1], fd, EVFILT_TIMER, EV_ADD | EV_ENABLE, 0, SET_TIMEOUT, NULL);

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
    std::cout << "Connection closed by server on socket fd " << fd << std::endl;
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
    Body &body = client.getRequest().getBody();
    char buffer;
    std::string chunkSize;
    std::string chunk;
    int size = 0;
    while (recv(client.getClientFd(), &buffer, 1, 0) > 0)
    {
        if (buffer == '\r')
        {
            recv(client.getClientFd(), &buffer, 1, 0);
            if (buffer == '\n')
            {
                size = std::stoi(chunkSize, 0, 16);
                if (size == 0)
                    break;
                while (recv(client.getClientFd(), &buffer, 1, 0) > 0)
                {
                    chunk.append(1, buffer);
                    if (size == chunk.size())
                        break;
                }
                body.getContent() += chunk;
                chunkSize.clear();
                chunk.clear();
            }
        }
        else
            chunkSize.append(1, buffer);
    }
    return true;
}

void Connections::sendResponse(Client &client, int clientFd)
{
    client.getResponse().handleResponse(client.getRequest());
    client.getWriteBuffer() = client.getResponse().getResponse();
    client.setWritePending(true);
    client.setReadPending(false);
    client.getReadBuffer().clear();
    this->setWriteEvent(clientFd);
}

void Connections::handleReadEvent(int clientFd)
{
    Client &client = clients.at(clientFd);
    Headers& header = client.getRequest().getHeaders();
    try
    {

        if (!ft_recv_header(clientFd, header.getRawHeaders()))
            return (removeClient(clientFd));
        header.parseHeader(client.getResponse());

        Body &body = client.getRequest().getBody();
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
                if (!body.readBody(clientFd, contentLength))
                    return (removeClient(clientFd));
                std::cout << "Body: " << body.getContent() << std::endl;
            }
        }
        bool isErrorResponse = client.getRequest().handleRequest(this->configFile, client.getResponse());

        if (client.getRequest().isComplete())
        {
            if (isErrorResponse)
                client.getResponse().handleResponse(client.getRequest());
            client.getWriteBuffer() = client.getResponse().getResponse();
            client.setWritePending(true);
            client.setReadPending(false);
            client.getReadBuffer().clear();
            this->setWriteEvent(clientFd);
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        std::cerr << "Error reading from client socket hi" << std::endl;
        sendResponse(client, clientFd);
        removeClient(clientFd);
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

        EV_SET(&changeList[1], clientFd, EVFILT_TIMER, EV_ADD | EV_ENABLE, 0, SET_TIMEOUT, NULL);
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
    EV_SET(&changeList[0], fd, EVFILT_TIMER, EV_ADD | EV_ENABLE, 0, 0, NULL);
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
