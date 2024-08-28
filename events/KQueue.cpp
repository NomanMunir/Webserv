#if defined(__APPLE__) || defined(__FreeBSD__)

#include "KQueue.hpp"
#include "../response/Response.hpp"
#include "../request/Request.hpp"

KQueue::KQueue()
{
    std::memset(this->events, 0, sizeof(this->events));
    this->kqueueFd = kqueue();
    if (this->kqueueFd == -1)
    {
        std::cerr << "KQueue::Error: " << strerror(errno) << std::endl;
        throw std::exception();
    }
}

KQueue &KQueue::operator=(const KQueue &k)
{
    if (this != &k)
        this->kqueueFd = k.kqueueFd;
    return *this;
}

KQueue::~KQueue() { close(this->kqueueFd); }

KQueue::KQueue(const KQueue &k) { this->kqueueFd = k.kqueueFd; }



void setNoneBlocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
    {
        std::cerr << "setNoneBlocking::Error: " << strerror(errno) << std::endl;
        close(fd);
        throw std::exception();
    }
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
    {
        std::cerr << "setNoneBlocking::Error: " << strerror(errno) << std::endl;
        close(fd);
        throw std::exception();
    }
}

void KQueue::addToQueue(int fd, EventType ev)
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
    else if (ev == TIMEOUT_EVENT)
    {
        EV_SET(&evSet, fd, EVFILT_TIMER, EV_ADD, 0, SET_TIMEOUT, NULL);
        fdState[fd].isTimeout = true;
    }
    else
        throw std::runtime_error("Error Invalid Event Type");

    if (kevent(this->kqueueFd, &evSet, 1, NULL, 0, NULL) == -1)
        std::cerr << "Error adding event: " << strerror(errno) << std::endl;
}

void KQueue::removeFromQueue(int fd, EventType ev)
{
    struct kevent evSet;

    if (ev == READ_EVENT)
    {
        if (fdState[fd].isRead)
        {
            EV_SET(&evSet, fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
            if (kevent(this->kqueueFd, &evSet, 1, NULL, 0, NULL) == -1)
                std::cerr << "Error removing event: " << strerror(errno) << std::endl;
            fdState[fd].isRead = false;
        }
    }
    else if (ev == WRITE_EVENT)
    {
        if (fdState[fd].isWrite)
        {
            std::cout << "Removing read event for fd: " << fd << std::endl;
            EV_SET(&evSet, fd, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
            if (kevent(this->kqueueFd, &evSet, 1, NULL, 0, NULL) == -1)
                std::cerr << "Error removing event: " << strerror(errno) << std::endl;
            fdState[fd].isWrite = false;
        }
    }
    else if (ev == TIMEOUT_EVENT)
    {
        if (fdState[fd].isTimeout)
        {
            EV_SET(&evSet, fd, EVFILT_TIMER, EV_DELETE, 0, 0, NULL);
            if (kevent(this->kqueueFd, &evSet, 1, NULL, 0, NULL) == -1)
                std::cerr << "Error removing event: " << strerror(errno) << std::endl;
            fdState[fd].isTimeout = false;
        }
    }
    else
        throw std::runtime_error("Error Invalid Event Type");

    if (!fdState[fd].isRead || !fdState[fd].isWrite || !fdState[fd].isTimeout)
        fdState.erase(fd);
}

int	KQueue::getNumOfEvents()
{
	struct timespec timeout;
	timeout.tv_sec = KEVENT_TIMEOUT_SEC;
	timeout.tv_nsec = 0;

	int nev = kevent(this->kqueueFd, NULL, 0, this->events, MAX_EVENTS, &timeout);
	return (nev);
}

EventInfo KQueue::getEventInfo(int i)
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
    else if (this->events[i].filter == EVFILT_TIMER)
        info.isTimeout = true;
    else
        info.isError = true;
    return (info);
}

#endif // defined(__APPLE__) || defined(__FreeBSD__)