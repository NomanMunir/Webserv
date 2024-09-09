#if defined(__APPLE__) || defined(__FreeBSD__)

#include "KQueuePoller.hpp"
#include <sys/event.h>
#include <sys/time.h>
#include <fcntl.h>
#include <cstring>

KQueuePoller::KQueuePoller()
{
    std::memset(this->events, 0, sizeof(this->events));
    this->kqueueFd = kqueue();
    if (this->kqueueFd == -1)
    {
        Logs::appendLog("ERROR", "[KQueuePoller]\t\t " + std::string(strerror(errno)));
        throw std::exception();
    }
}

KQueuePoller::~KQueuePoller() { close(this->kqueueFd); }

void KQueuePoller::addToQueue(int fd, EventType ev)
{
    struct kevent evSet;
    setNoneBlocking(fd);

    if (ev == READ_EVENT)
    {
        EV_SET(&evSet, fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
        fdState[fd].isRead = true;
    }
    else if (ev == WRITE_EVENT)
    {
        EV_SET(&evSet, fd, EVFILT_WRITE, EV_ADD, 0, 0, NULL);
        fdState[fd].isWrite = true;
    }
    else
    {
        Logs::appendLog("ERROR", "[addToQueue]\t\tInvalid Event Type");
        return;
    }

    if (kevent(this->kqueueFd, &evSet, 1, NULL, 0, NULL) == -1)
    {
        Logs::appendLog("ERROR", "[addToQueue]\t\tError Adding Event " + std::string(strerror(errno)));
    }
}

void KQueuePoller::removeFromQueue(int fd, EventType ev)
{
    struct kevent evSet;

    if (ev == READ_EVENT && fdState[fd].isRead)
    {
        EV_SET(&evSet, fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
        if (kevent(this->kqueueFd, &evSet, 1, NULL, 0, NULL) == -1)
        {
            Logs::appendLog("ERROR", "[removeFromQueue]\t\tError Removing READ Event " + std::to_string(fd) + " " + std::string(strerror(errno)));
        }
        fdState[fd].isRead = false;
    }
    else if (ev == WRITE_EVENT && fdState[fd].isWrite)
    {
        EV_SET(&evSet, fd, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
        if (kevent(this->kqueueFd, &evSet, 1, NULL, 0, NULL) == -1)
        {
            Logs::appendLog("ERROR", "[removeFromQueue]\t\tError Removing WRITE Event " + std::to_string(fd) + " " + std::string(strerror(errno)));
        }
        fdState[fd].isWrite = false;
    }

    if (!fdState[fd].isRead && !fdState[fd].isWrite)
    {
        fdState.erase(fd);
    }
}

int KQueuePoller::getNumOfEvents()
{
    struct timespec timeout;
    timeout.tv_sec = 1;
    timeout.tv_nsec = 0;
    int nev = kevent(this->kqueueFd, NULL, 0, this->events, MAX_EVENTS, &timeout);
    return nev;
}

EventInfo KQueuePoller::getEventInfo(int i)
{
    EventInfo info;
    std::memset(&info, 0, sizeof(info));

    info.fd = this->events[i].ident;

    if (this->events[i].flags & EV_ERROR)
        info.isError = true;
    else if (this->events[i].flags & EV_EOF)
        info.isEOF = true;
    else if (this->events[i].filter == EVFILT_READ)
        info.isRead = true;
    else if (this->events[i].filter == EVFILT_WRITE)
        info.isWrite = true;
    else
        info.isError = true;
    return info;
}

#endif // __APPLE__ || __FreeBSD__
