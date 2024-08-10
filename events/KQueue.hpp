#ifndef KQUEUE_HPP
#define KQUEUE_HPP

#if defined(__APPLE__) || defined(__FreeBSD__)

#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <sys/event.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <unordered_map>

#include "../parsing/Parser.hpp"
#include "../Client.hpp"

#define MAX_EVENTS 100

class Connections {
private:

    std::vector<int> serverSockets;
    int kqueueFd;
    struct kevent changeList[MAX_EVENTS];
    struct kevent events[MAX_EVENTS];
    std::unordered_map<int, Client> clients;
    Parser configFile;

    void setNonBlocking(int fd);
    void setClient(int fd);
    void removeClient(int fd);
    bool peekRequest(int clientSocket);
    bool handleClient(int clientSocket, Parser &configFile);
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

#endif // __APPLE__ || __FreeBSD__

#endif // KQUEUE_HPP