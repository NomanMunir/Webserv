#if defined(__linux__)

#include "EpollPoller.hpp"

EpollPoller::EpollPoller()
{
    this->epollFd = epoll_create1(0);
    if (this->epollFd < 0)
		throw std::runtime_error("Failed to create epoll instance");
}

EpollPoller::~EpollPoller() { close(this->epollFd); }

void EpollPoller::addToQueue(int fd, EventType event)
{
	struct epoll_event	epollEvent;
	std::memset(&epollEvent, 0, sizeof(epollEvent));

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
        Logs::appendLog("ERROR", "[addToQueue]\t\tError Adding Event " + filterType + " " + std::string(strerror(errno)));
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
	
	if (this->fdState.find(fd) == this->fdState.end())	return ;
	else
	{
		if (event == READ_EVENT && this->fdState[fd].isRead)
		{
			op = EPOLL_CTL_DEL;
			epollEvent.events = EPOLLIN | EPOLLHUP | EPOLLRDHUP;
			filterType = "READ EVENT";
			this->fdState[fd].isRead = false;
		}
		else if (event == WRITE_EVENT && this->fdState[fd].isWrite)
		{
			op = EPOLL_CTL_MOD;
			epollEvent.events = EPOLLIN | EPOLLHUP | EPOLLRDHUP;
			filterType = "WRITE EVENT";
			this->fdState[fd].isWrite = false;
		}
		// else
		// {
		// 	Logs::appendLog("ERROR", "[removeFromQueue]\t\tInvalid Event Type");
		// 	if (!this->fdState[fd].isRead && !this->fdState[fd].isWrite)
		// 		this->fdState.erase(fd);		
		// 	return ;
		// }
	}

	if (epoll_ctl(this->epollFd, op, fd, &epollEvent) < 0)
        Logs::appendLog("ERROR", "[removeFromQueue]\t\tError Removing Event " + std::string(strerror(errno)));
	else
        Logs::appendLog("INFO", "[removeFromQueue]\t\tRemoved Event " + filterType + " from " + std::to_string(fd));

	if (!this->fdState[fd].isRead && !this->fdState[fd].isWrite)
		this->fdState.erase(fd);
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

    if (this->events[i].events & EPOLLERR)
        info.isError = true;
    else if (this->events[i].events & EPOLLHUP)
        info.isEOF = true;
    else if (this->events[i].events & EPOLLIN)
        info.isRead = true;
    else if (this->events[i].events & EPOLLOUT)
        info.isWrite = true;
	else
        info.isError = true;
    return info;
}

#endif // __linux__
