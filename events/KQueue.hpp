#ifndef KQUEUE_HPP
#define KQUEUE_HPP

#if defined(__APPLE__) || defined(__FreeBSD__)

#define KEVENT_TIMEOUT_SEC 5
#define SET_TIMEOUT 300000 // 300 ms

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
        
    public:
        KQueue();
        ~KQueue();
        KQueue(const KQueue &c);
        KQueue &operator=(const KQueue &c);

        struct kevent events[MAX_EVENTS];

        void addToQueue(int fd, EventType type);
        void removeFromQueue(int fd, EventType type);
        int	getNumOfEvents();

};

#endif // __APPLE__ || __FreeBSD__

#endif // KQUEUE_HPP