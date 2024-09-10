#ifndef EPOLL_POLLER_HPP
#define EPOLL_POLLER_HPP

#if defined(__linux__)

#include "EventPoller.hpp"
#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <map>

class EpollPoller : public EventPoller
{
private:
    int epollFd;
    struct epoll_event events[MAX_EVENTS];
    std::map<int, EventInfo> fdState;

public:
    EpollPoller();
    ~EpollPoller();

    void addToQueue(int fd, EventType event);
    void removeFromQueue(int fd, EventType event);
    int getNumOfEvents();
    EventInfo getEventInfo(int i);
};

#endif // __linux__

#endif // EPOLL_POLLER_HPP
