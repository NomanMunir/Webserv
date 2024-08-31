#ifndef EPOLL_HPP
#define EPOLL_HPP

#if defined(__linux__)

#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <map>
#include <cstring>
#include <stdexcept>

#include "../parsing/Parser.hpp"
#include "../Client.hpp"

#define MAX_EVENTS 100
#define EPOLL_TIMEOUT 5000 // in milliseconds

enum EventType
{
    READ_EVENT = 0,
    WRITE_EVENT = 1,
    TIMEOUT_EVENT = 2
};

struct EventInfo
{
    int     fd;
    bool    isRead;
    bool    isWrite;
    bool    isError;
    bool    isHup;
};

class Epoll
{
private:
    int epollFd;
    struct epoll_event events[MAX_EVENTS];

public:
    Epoll();
    ~Epoll();
    Epoll(const Epoll &other);
    Epoll &operator=(const Epoll &other);

    std::map<int, EventInfo> fdState;

    void addToQueue(int fd, EventType type);
    void modifyInQueue(int fd, EventType type);
    void removeFromQueue(int fd);

    int getNumOfEvents();
    EventInfo getEventInfo(int i);

private:
    void setNonBlocking(int fd);
};

#endif // __linux__

#endif // EPOLL_HPP
