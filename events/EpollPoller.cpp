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

void EpollPoller::addToQueue(int fd, EventType event)
{
	struct epoll_event	epollEvent;
	memset(&epollEvent, 0, sizeof(epollEvent));

    setNoneBlocking(fd);
	epollEvent.data.fd = fd;
	epollEvent.events = 0;
	int op = 0;

	std::string		filterType = "UNKNOWN EVENT";

	if (this->fdState.find(fd) == this->fdState.end())
	{
		op = EPOLL_CTL_ADD;

		if (event == READ_EVENT)
		{
			epollEvent.events = EPOLLIN | EPOLLHUP | EPOLLRDHUP;
		    this->fdState[fd].isRead = true;
			filterType = "READ EVENT";
		}
		else if (event == WRITE_EVENT)
		{
			epollEvent.events = EPOLLOUT | EPOLLHUP | EPOLLRDHUP;
		    this->fdState[fd].isWrite = true;
			filterType = "WRITE EVENT";
		}
	}
	else
	{
		op = EPOLL_CTL_MOD;
		if (event == READ_EVENT)
		{
			epollEvent.events = EPOLLIN | EPOLLOUT | EPOLLHUP | EPOLLRDHUP;
            this->fdState[fd].isRead = true;
			filterType = "READ EVENT";
		}
		else if (event == WRITE_EVENT)
		{
			epollEvent.events = EPOLLIN | EPOLLOUT | EPOLLHUP | EPOLLRDHUP;
            this->fdState[fd].isWrite = true;
			filterType = "WRITE EVENT";
		}
	}
	if (epoll_ctl(this->epollFd, op, fd, &epollEvent) < 0)
        Logs::appendLog("ERROR", "[addToQueue]\t\tError Adding Event " + std::string(strerror(errno)));
    else
        Logs::appendLog("INFO", "[addToQueue]\t\tAdded Event " + filterType + " to " + std::to_string(fd));
}

void EpollPoller::removeFromQueue(int fd, EventType event)
{
	struct epoll_event	epollEvent;
	memset(&epollEvent, 0, sizeof(epollEvent));

	epollEvent.events = 0;
	epollEvent.data.fd = fd;
	int op = 0;

	std::string		filterType = "UNKNOWN EVENT";
	
	if (this->fdState.find(fd) == this->fdState.end())
	{
        Logs::appendLog("ERROR", "[removeFromQueue]\t\tEvent not found for fd " + std::to_string(fd));
	}
	else
	{
		if (event == READ_EVENT)
		{
			op = EPOLL_CTL_DEL;
			epollEvent.events = EPOLLIN | EPOLLHUP | EPOLLRDHUP;
			filterType = "READ EVENT";
			this->fdState.erase(fd);
		}
		else if (event == WRITE_EVENT)
		{
			op = EPOLL_CTL_MOD;
			epollEvent.events = EPOLLIN | EPOLLHUP | EPOLLRDHUP;
			filterType = "WRITE EVENT";
		}
	}

	if (epoll_ctl(this->epollFd, op, fd, &epollEvent) < 0)
        Logs::appendLog("ERROR", "[removeFromQueue]\t\tError Removing Event " + std::string(strerror(errno)));
	else
        Logs::appendLog("INFO", "[removeFromQueue]\t\tRemoved Event " + filterType + " from " + std::to_string(fd));
	
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

    // time_t now = time(NULL);

    // if (now - lastActivity[info.fd] > CLIENT_TIMEOUT) {
    //     info.isTimeout = true;
    //     return info;
    // }

    // lastActivity[info.fd] = now;
    

    if (this->events[i].events & EPOLLERR)
    {
        info.isError = true;
    }
    else if (this->events[i].events & EPOLLHUP)
    {
        info.isError = true;
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
