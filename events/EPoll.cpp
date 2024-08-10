#ifdef __linux__

#include "EPoll.hpp"
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
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
    {
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

bool Connections::addClient(int serverSocket)
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

            // Check if it's a timer event
            if (timerfd_map.count(eventList[i].data.fd) > 0)
            {
                handleTimeoutEvent(eventList[i].data.fd);
                continue;
            }

            if (eventList[i].events & EPOLLIN)
            {
                if (std::find(serverSockets.begin(), serverSockets.end(), eventList[i].data.fd) != serverSockets.end())
                {
                    if (!addClient(eventList[i].data.fd))
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
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        this->setWriteEvent(clientFd);
        removeClient(clientFd);
    }
}

void Connections::handleWriteEvent(int clientFd)
{
    Client &client = clients.at(clientFd);
    if (!client.isWritePending()) return;

    ssize_t bytesWritten = send(clientFd, client.getWriteBuffer().c_str(), client.getWriteBuffer().size(), 0);
    if (bytesWritten < 0)
    {
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

void Connections::removeWriteEvent(int clientFd)
{
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = clientFd;

    if (epoll_ctl(epollFd, EPOLL_CTL_MOD, clientFd, &ev) == -1)
    {
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

    if (epoll_ctl(this->epollFd, EPOLL_CTL_MOD, clientFd, &ev) == -1)
    {
        std::cerr << "Error setting write event: " << strerror(errno) << std::endl;
        removeClient(clientFd);  // Handle error by removing client
    }
}

void Connections::setTimeout(int fd)
{
    int tfd = timerfd_create(CLOCK_MONOTONIC, 0);
    if (tfd == -1)
    {
        std::cerr << "Error creating timerfd: " << strerror(errno) << std::endl;
        removeClient(fd);
        return;
    }

    struct itimerspec new_value;
    new_value.it_value.tv_sec = SET_TIMEOUT / 1000;        // Timeout in seconds
    new_value.it_value.tv_nsec = (SET_TIMEOUT % 1000) * 1000000; // Timeout in nanoseconds
    new_value.it_interval.tv_sec = 0;
    new_value.it_interval.tv_nsec = 0; // Non-periodic timer

    if (timerfd_settime(tfd, 0, &new_value, NULL) == -1)
    {
        std::cerr << "Error setting timerfd time: " << strerror(errno) << std::endl;
        close(tfd);
        removeClient(fd);
        return;
    }

    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = tfd;

    if (epoll_ctl(epollFd, EPOLL_CTL_ADD, tfd, &ev) == -1)
    {
        std::cerr << "Error adding timerfd to epoll: " << strerror(errno) << std::endl;
        close(tfd);
        removeClient(fd);
        return;
    }

    timerfd_map[fd] = tfd; // Map client FD to timer FD
}

void Connections::handleTimeoutEvent(int clientFd)
{
    int timerFd = timerfd_map[clientFd];

    uint64_t expirations;
    if (read(timerFd, &expirations, sizeof(expirations)) < 0)
    {
        std::cerr << "Error reading from timerfd: " << strerror(errno) << std::endl;
    }

    std::cout << "Timeout event on client: " << clientFd << std::endl;
    removeClient(clientFd);
}

void Connections::removeClient(int fd)
{
    // Remove the associated timerfd if it exists
    if (timerfd_map.find(fd) != timerfd_map.end())
    {
        int tfd = timerfd_map[fd];
        epoll_ctl(epollFd, EPOLL_CTL_DEL, tfd, NULL);
        close(tfd);
        timerfd_map.erase(fd);
    }

    if (epoll_ctl(epollFd, EPOLL_CTL_DEL, fd, NULL) == -1)
        std::cerr << "Error: " << strerror(errno) << std::endl;
    clients.erase(fd);
    close(fd);
    std::cout << "Connection closed by server on socket fd " << fd << std::endl;
}

#endif // __linux__
