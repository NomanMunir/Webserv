#ifndef CONNECTIONS_HPP
#define CONNECTIONS_HPP

#include <iostream>
#include <exception>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include "response/Response.hpp"
#include "request/Request.hpp"

#define MAX_EVENTS 100

class Connections
{
private:
    int serverSocket;
    struct sockaddr_in serverAddr;
    int epollFd;
    struct epoll_event event;
    struct epoll_event events[MAX_EVENTS];

    void setClient(int fd);
    void removeClient(int fd);
    bool addClient();
    bool peekRequest(int clientSocket);
    bool handleClient(int clientSocket, Parser &configFile);

public:
    Connections(int fd);
    ~Connections();

    void loop(Parser &configFile);
};

#endif // CONNECTIONS_HPP