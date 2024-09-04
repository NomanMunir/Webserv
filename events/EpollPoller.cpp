#if defined(__linux__)

#include "EpollPoller.hpp"
#define CLIENT_TIMEOUT 30  // 30 seconds timeout for clients

EpollPoller::EpollPoller()
{
    this->epollFd = epoll_create1(0);
    if (this->epollFd == -1)
    {
        Logs::appendLog("ERROR", "[EpollPoller]\t\t " + std::string(strerror(errno)));
        throw std::exception();
    }
}

EpollPoller::~EpollPoller() { close(this->epollFd); }

void setNoneBlocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
    {
        Logs::appendLog("ERROR", "[setNoneBlocking]\t\t " + std::string(strerror(errno)));
        close(fd);
        throw std::exception();
    }
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
    {
        Logs::appendLog("ERROR", "[setNoneBlocking]\t\t " + std::string(strerror(errno)));
        close(fd);
        throw std::exception();
    }
}

void EpollPoller::addToQueue(int fd, EventType ev)
{
    struct epoll_event event;
    std::memset(&event, 0, sizeof(event));
    setNoneBlocking(fd);

    event.data.fd = fd;
    event.events = EPOLL_CTL_MOD;
    if (ev == READ_EVENT)
    {
        event.events = EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLHUP;
        fdState[fd].isRead = true;
    }
    else if (ev == WRITE_EVENT)
    {
        event.events = EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLHUP;
        fdState[fd].isWrite = true;
    }
    else if (ev == TIMEOUT_EVENT)
    {
        lastActivity[fd] = time(NULL);
        fdState[fd].isTimeout = true;
        return ;
    }
    else
    {
        throw std::runtime_error("[addToQueue]\t\t Invalid Event Type");
    }

    if (epoll_ctl(this->epollFd, EPOLL_CTL_ADD, fd, &event) == -1)
    {
        Logs::appendLog("ERROR", "[addToQueue]\t\tError Adding Event " + std::string(strerror(errno)));
    }
    lastActivity[fd] = time(NULL);
}

void EpollPoller::removeFromQueue(int fd, EventType ev)
{
    if (epoll_ctl(this->epollFd, EPOLL_CTL_DEL, fd, NULL) == -1)
    {
        Logs::appendLog("ERROR", "[removeFromQueue]\t\tError Removing Event " + std::string(strerror(errno)));
    }
    fdState.erase(fd);
    lastActivity.erase(fd);
}

int EpollPoller::getNumOfEvents()
{
    int nev = epoll_wait(this->epollFd, this->events, MAX_EVENTS, EPOLLEVENT_TIMEOUT_SEC);
    return nev;
}

EventInfo EpollPoller::getEventInfo(int i)
{
    EventInfo info;
    std::memset(&info, 0, sizeof(info));

    info.fd = this->events[i].data.fd;

    time_t now = time(NULL);

    if (now - lastActivity[info.fd] > CLIENT_TIMEOUT) {
        info.isTimeout = true;
        return info;
    }

    lastActivity[info.fd] = now;
    

    if (this->events[i].events & EPOLLERR)
    {
        info.isError = true;
    }
    else if (this->events[i].events & EPOLLHUP)
    {
        info.isEOF = true;
    }
    else if (this->events[i].events & EPOLLIN)
    {
        info.isRead = true;
    }
    else if (this->events[i].events & EPOLLOUT)
    {
        info.isWrite = true;
    }
    return info;
}

#endif // __linux__
