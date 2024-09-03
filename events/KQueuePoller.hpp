#ifndef KQUEUE_POLLER_HPP
#define KQUEUE_POLLER_HPP

#if defined(__APPLE__) || defined(__FreeBSD__)

#include "EventPoller.hpp"

#define KEVENT_TIMEOUT_SEC 5
#define SET_TIMEOUT 300000 // 300 ms
#define MAX_EVENTS 100

class KQueuePoller : public EventPoller
{
private:
    int kqueueFd;
    struct kevent events[MAX_EVENTS];
    std::map<int, EventInfo> fdState;

public:
    KQueuePoller();
    ~KQueuePoller();

    void addToQueue(int fd, EventType type);
    void removeFromQueue(int fd, EventType type);
    int getNumOfEvents();
    EventInfo getEventInfo(int i);
};

#endif // __APPLE__ || __FreeBSD__

#endif // KQUEUE_POLLER_HPP
