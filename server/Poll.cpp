#include "Poll.hpp"


Poll::Poll()
{

}

Poll::~Poll()
{

}
bool Poll::addSocket(int fd, short events) 
{
    pollfd pfd;
    bzero(&pfd, sizeof(pfd));
    if(fd < 0)
    {
        perror("Error: Invalid file descriptor");
        return false;
    }
    pfd.fd = fd;
    pfd.events = events;
    socketsFd.push_back(pfd);
    return true;
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