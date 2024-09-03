#ifndef KQUEUE_HPP
#define KQUEUE_HPP

#if defined(__APPLE__) || defined(__FreeBSD__)

#define KEVENT_TIMEOUT_SEC 5
#define SET_TIMEOUT 300000 // 300 ms


#include <sys/event.h>
#include <sys/time.h>
#include <dirent.h>
#include <fcntl.h>

#include "../parsing/Parser.hpp"
#include "../Client.hpp"
#include <map>
#include "../utils/Logs.hpp"
#define MAX_EVENTS 100

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
        std::map<int, EventInfo> fdState;

        void addToQueue(int fd, EventType type);
        void removeFromQueue(int fd, EventType type);

        int	getNumOfEvents();
        EventInfo getEventInfo(int i);
};

#endif // __APPLE__ || __FreeBSD__

#endif // KQUEUE_HPP