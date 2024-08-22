#ifndef KQUEUE_HPP
#define KQUEUE_HPP

#if defined(__APPLE__) || defined(__FreeBSD__)

#define KEVENT_TIMEOUT_SEC 5

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

enum EventType
{
    READ_EVENT = 0,
    WRITE_EVENT = 1,
    TIMEOUT_EVENT = 2
};

class KQueue 
{
    private:
        int kqueueFd;

        // void setNonBlocking(int fd);
        // void setClient(int fd);
        // void removeClient(int fd);
        // void setTimeout(int fd);
        // void setWriteEvent(int clientFd);
        // void removeWriteEvent(int clientFd);
        // void removeReadEvent(int clientFd);
        // void removeTimeEvent(int clientFd);



        // void setServer(int fd);
        // bool addClient(int serverSocket);

        // void handleReadEvent(int clientFd);
        // void handleWriteEvent(int clientFd);
        // void handleTimeoutEvent(int clientFd);
        
    public:
        KQueue();
        struct kevent events[MAX_EVENTS];

        void addToQueue(int fd, EventType type);
        void removeFromQueue(int fd, EventType type);
        int	getNumOfEvents();




        // ~KQueue();
        // KQueue(const KQueue &c);
        // KQueue &operator=(const KQueue &c);

};

#endif // __APPLE__ || __FreeBSD__

#endif // KQUEUE_HPP