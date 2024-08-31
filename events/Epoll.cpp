#if defined(__linux__)

#include "Epoll.hpp"
#include "../response/Response.hpp"
#include "../request/Request.hpp"

Epoll::Epoll()
{
    this->epollFd = epoll_create1(0);
    if (this->epollFd == -1)
    {
        std::cerr << "Epoll::Error creating epoll instance: " << strerror(errno) << std::endl;
        throw std::runtime_error("Failed to create epoll instance");
    }
    std::memset(this->events, 0, sizeof(this->events));
}

Epoll::~Epoll()
{
    close(this->epollFd);
}

Epoll::Epoll(const Epoll &other)
{
    this->epollFd = other.epollFd;
    std::memcpy(this->events, other.events, sizeof(this->events));
    this->fdState = other.fdState;
}

Epoll &Epoll::operator=(const Epoll &other)
{
    if (this != &other)
    {
        this->epollFd = other.epollFd;
        std::memcpy(this->events, other.events, sizeof(this->events));
        this->fdState = other.fdState;
    }
    return *this;
}

void Epoll::setNonBlocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
    {
        std::cerr << "Epoll::setNonBlocking::Error getting flags: " << strerror(errno) << std::endl;
        close(fd);
        throw std::runtime_error("Failed to get file descriptor flags");
    }
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
    {
        std::cerr << "Epoll::setNonBlocking::Error setting non-blocking: " << strerror(errno) << std::endl;
        close(fd);
        throw std::runtime_error("Failed to set non-blocking mode");
    }
}

void Epoll::addToQueue(int fd, EventType type)
{
    setNonBlocking(fd);

    struct epoll_event event;
    std::memset(&event, 0, sizeof(event));
    event.data.fd = fd;

    if (type == READ_EVENT)
    {
        event.events = EPOLLIN | EPOLLET;
        fdState[fd].isRead = true;
    }
    else if (type == WRITE_EVENT)
    {
        event.events = EPOLLOUT | EPOLLET;
        fdState[fd].isWrite = true;
    }
    else
    {
        throw std::runtime_error("Epoll::addToQueue::Invalid Event Type");
    }

    if (epoll_ctl(this->epollFd, EPOLL_CTL_ADD, fd, &event) == -1)
    {
        std::cerr << "Epoll::addToQueue::Error adding fd to epoll: " << strerror(errno) << std::endl;
        throw std::runtime_error("Failed to add file descriptor to epoll");
    }
}

void Epoll::modifyInQueue(int fd, EventType type)
{
    struct epoll_event event;
    std::memset(&event, 0, sizeof(event));
    event.data.fd = fd;

    if (type == READ_EVENT)
    {
        event.events = EPOLLIN | EPOLLET;
        fdState[fd].isRead = true;
        fdState[fd].isWrite = false;
    }
    else if (type == WRITE_EVENT)
    {
        event.events = EPOLLOUT | EPOLLET;
        fdState[fd].isWrite = true;
        fdState[fd].isRead = false;
    }
    else
    {
        throw std::runtime_error("Epoll::modifyInQueue::Invalid Event Type");
    }

    if (epoll_ctl(this->epollFd, EPOLL_CTL_MOD, fd, &event) == -1)
    {
        std::cerr << "Epoll::modifyInQueue::Error modifying fd in epoll: " << strerror(errno) << std::endl;
        throw std::runtime_error("Failed to modify file descriptor in epoll");
    }
}

void Epoll::removeFromQueue(int fd)
{
    if (epoll_ctl(this->epollFd, EPOLL_CTL_DEL, fd, NULL) == -1)
    {
        std::cerr << "Epoll::removeFromQueue::Error removing fd from epoll: " << strerror(errno) << std::endl;
        throw std::runtime_error("Failed to remove file descriptor from epoll");
    }
    fdState.erase(fd);
    close(fd);
}

int Epoll::getNumOfEvents()
{
    int eventCount = epoll_wait(this->epollFd, this->events, MAX_EVENTS, EPOLL_TIMEOUT);
    if (eventCount == -1)
    {
        std::cerr << "Epoll::getNumOfEvents::Error during epoll_wait: " << strerror(errno) << std::endl;
        throw std::runtime_error("epoll_wait failed");
    }
    return eventCount;
}

EventInfo Epoll::getEventInfo(int i)
{
    EventInfo info;
    std::memset(&info, 0, sizeof(info));

    info.fd = this->events[i].data.fd;

    if (events[i].events & EPOLLERR)
        info.isError = true;
    if (events[i].events & EPOLLHUP)
        info.isHup = true;
    if (events[i].events & EPOLLIN)
        info.isRead = true;
    if (events[i].events & EPOLLOUT)
        info.isWrite = true;

    return info;
}

#endif // __linux__
