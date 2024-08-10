#ifndef EPOLL_HPP
#define EPOLL_HPP

#if defined(__linux__)

#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <sys/epoll.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <unordered_map>
#include <sys/timerfd.h>

#include "../parsing/Parser.hpp"
#include "../Client.hpp"

#define MAX_EVENTS 100
#define SET_TIMEOUT 300000

class Connections {
private:
    std::unordered_map<int, int> timerfd_map;
    std::vector<int> serverSockets;
    int epollFd;
    struct epoll_event eventList[MAX_EVENTS];
    std::unordered_map<int, Client> clients;
    Parser configFile;

    void setNonBlocking(int fd);
    void setClient(int fd);
    void removeClient(int fd);
    void setTimeout(int fd);
    void setWriteEvent(int clientFd);
    void removeWriteEvent(int clientFd);

    void setServer(int fd);
    bool addClient(int serverSocket);

    void handleReadEvent(int clientFd);
    void handleWriteEvent(int clientFd);
    void handleTimeoutEvent(int clientFd);

public:
    Connections(std::vector<int> fds, Parser &configFile);
    ~Connections();
    Connections(const Connections &c);
    Connections &operator=(const Connections &c);

    void loop();
};

#endif // __linux__
#endif // EPOLL_HPP
