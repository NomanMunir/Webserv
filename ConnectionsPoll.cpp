#include "ConnectionsPoll.hpp"

Connections::~Connections() {}

Connections::Connections(int fd) : serverSocket(fd)
{
    this->epollFd = epoll_create1(0);
    if (this->epollFd == -1)
    {
        std::cerr << "Error: " << strerror(errno) << std::endl;
        throw std::exception();
    }
    setClient(fd);
}

static void setNonBlocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

void Connections::removeClient(int fd)
{
    epoll_ctl(this->epollFd, EPOLL_CTL_DEL, fd, NULL);
    std::cout << "Connection closed by client on socket fd " << fd << std::endl;
    close(fd);
}

void Connections::setClient(int fd)
{
    setNonBlocking(fd);
    this->event.data.fd = fd;
    this->event.events = EPOLLIN;
    if (epoll_ctl(this->epollFd, EPOLL_CTL_ADD, fd, &this->event) == -1)
    {
        std::cerr << "Error: " << strerror(errno) << std::endl;
        close(fd);
        throw std::exception();
    }
}

bool Connections::peekRequest(int clientSocket)
{
    char buffer[1];
    ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer), MSG_PEEK);
    if (bytesRead < 0)
    {
        perror("Error reading from client socket");
        return true;
    }
    else if (bytesRead == 0)
    {
        return true;
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
            return false;
        }
        Response response(clientSocket);
        Request request(clientSocket, configFile, response);

        // Handle the response and send it to the client
        response.handleResponse(request);
        std::string clientResponse = response.getResponse();
        if (!responseClient(clientSocket, clientResponse))
            removeClient(clientSocket);
        if (response.getIsConClosed())
        {
            removeClient(clientSocket);
            return false;
        }

        std::string keepAlive = request.getHeaders().getValue("Connection");
        if (keepAlive != "keep-alive")
            removeClient(clientSocket);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error handling request: " << e.what() << std::endl;
        removeClient(clientSocket);
        return false;
    }
    return true;
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
    setClient(clientSocket);
    return true;
}

void Connections::loop(Parser &configFile)
{
    while (true)
    {
        int nev = epoll_wait(this->epollFd, this->events, MAX_EVENTS, -1);
        if (nev == -1)
        {
            std::cerr << "Error: " << strerror(errno) << std::endl;
            throw std::exception();
        }
        for (int i = 0; i < nev; i++)
        {
            if (this->events[i].data.fd == this->serverSocket)
            {
                if (!this->addClient())
                    continue;
            }
            else if (this->events[i].events & EPOLLIN)
            {
                if (!this->handleClient(this->events[i].data.fd, configFile))
                    continue;
            }
        }
    }
}
