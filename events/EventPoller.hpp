#ifndef EVENT_POLLER_HPP
#define EVENT_POLLER_HPP

#include "../utils/Logs.hpp"
#include <map>
#include <unistd.h>
#include <sys/event.h>
#include <sys/time.h>
#include <fcntl.h>
#include <cstring>

enum EventType
{
    READ_EVENT = 0,
    WRITE_EVENT = 1,
    TIMEOUT_EVENT = 2
};

struct EventInfo
{
    int             fd;
    bool            isTimeout;
    bool            isRead;
    bool            isWrite;
    bool            isEOF;
    bool            isError;
};

class EventPoller
{
    public:
        virtual ~EventPoller() {}

        virtual void addToQueue(int fd, EventType type) = 0;
        virtual void removeFromQueue(int fd, EventType type) = 0;
        virtual int getNumOfEvents() = 0;
        virtual EventInfo getEventInfo(int i) = 0;
};

#endif // EVENT_POLLER_HPP
