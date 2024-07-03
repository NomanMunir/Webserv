#include "Poll.hpp"


Poll::Poll()
{

}

Poll::~Poll()
{

}
void Poll::addSocket(int fd, short events) 
{
pollfd pfd;
pfd.fd = fd;
pfd.events = events;
socketsFd.push_back(pfd);
}

int Poll::waitForEvents(int timeout) 
{
return poll(socketsFd.data(), socketsFd.size(), timeout);
}

pollfd Poll::getEvent(int index) 
{
return socketsFd[index];
}

void Poll::removeSocket(int index) 
{
socketsFd.erase(socketsFd.begin() + index);
}

size_t Poll::getSocketsCount() const 
{
    return socketsFd.size();
}