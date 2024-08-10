#if defined(__linux__)
#include "Epoll.hpp"
#include "../response/Response.hpp"
#include "../request/Request.hpp"

void Connections::setServer(int fd)
{
    setNonBlocking(fd);
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = fd;

    if (epoll_ctl(epollFd, EPOLL_CTL_ADD, fd, &ev) == -1)
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
    this->epollFd = epoll_create1(0);
    if (this->epollFd == -1)
    {
        std::cerr << "Error: " << strerror(errno) << std::endl;
        throw std::exception();
    }

    for (size_t i = 0; i < serverSockets.size(); i++)
        setServer(serverSockets[i]);
}

Connections::Connections(const Connections &c)
    : serverSockets(c.serverSockets),
      epollFd(c.epollFd),
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
    epollFd = c.epollFd;
    configFile = c.configFile;
    for (size_t i = 0; i < serverSockets.size(); i++) 
        setServer(serverSockets[i]);
    return *this;
}

Connections::~Connections()
{
    for (size_t i = 0; i < serverSockets.size(); i++) 
        close(serverSockets[i]);
    close(epollFd);
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
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLONESHOT;
    ev.data.fd = fd;

    if (epoll_ctl(epollFd, EPOLL_CTL_ADD, fd, &ev) == -1)
    {
        std::cerr << "Error: " << strerror(errno) << std::endl;
        close(fd);
        throw std::exception();
    }
    clients[fd] = Client(fd);
}

void Connections::removeClient(int fd) 
{
    if (epoll_ctl(epollFd, EPOLL_CTL_DEL, fd, NULL) == -1)
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
    return true;
}

void Connections::loop() 
{
    while (true) 
    {
        int nev = epoll_wait(this->epollFd, this->eventList, MAX_EVENTS, -1);
        if (nev == -1)
        {
            std::cerr << "Error: " << strerror(errno) << std::endl;
        }

        for (int i = 0; i < nev; i++) 
        {
            if (eventList[i].events & EPOLLERR) 
            {
                std::cerr << "EPOLLERR: " << strerror(errno) << std::endl;
                continue;
            }

            if (eventList[i].events & EPOLLIN)
            {
                if (std::find(serverSockets.begin(), serverSockets.end(), eventList[i].data.fd) != serverSockets.end()) 
                {
                    if(!addClient(eventList[i].data.fd))
                        continue;
                }
                else
                {
                    std::cout << "Read event on client: " << eventList[i].data.fd << std::endl;
                    handleReadEvent(eventList[i].data.fd);
                }
            }
            else if (eventList[i].events & EPOLLOUT)
            {
                std::cout << "Write event on client: " << eventList[i].data.fd << std::endl;
                handleWriteEvent(eventList[i].data.fd);
            }
        }
    }
}

void Connections::handleReadEvent(int clientFd)
{
    try
    {
        Client &client = clients.at(clientFd);
        client.readFromSocket(this->configFile);
        this->setWriteEvent(clientFd);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        this->setWriteEvent(clientFd);
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

    client.getWriteBuffer().erase(0, bytesWritten);
    if (client.getWriteBuffer().empty()) 
    {
        client.setWritePending(false);
        removeWriteEvent(clientFd);

        if (!client.isKeepAlive()) 
        {
            std::cout << "Closed because of IsKeepAlive " << clientFd << std::endl;
            removeClient(clientFd);
            return;
        }

        struct epoll_event ev;
        ev.events = EPOLLIN | EPOLLONESHOT;
        ev.data.fd = clientFd;

        if (epoll_ctl(epollFd, EPOLL_CTL_MOD, clientFd, &ev) == -1)
        {
            std::cerr << "Error resetting keep-alive: " << strerror(errno) << std::endl;
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
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = clientFd;

    if (epoll_ctl(epollFd, EPOLL_CTL_MOD, clientFd, &ev) == -1) {
        std::cerr << "Error removing write event: " << strerror(errno) << std::endl;
        close(clientFd);
        clients.erase(clientFd);
    }
}

void Connections::setWriteEvent(int clientFd) 
{
    struct epoll_event ev;
    ev.events = EPOLLOUT | EPOLLONESHOT;
    ev.data.fd = clientFd;
    
    if (epoll_ctl(this->epollFd, EPOLL_CTL_MOD, clientFd, &ev) ==

#endif // __linux__