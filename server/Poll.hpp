#ifndef POLL_HPP
# define POLL_HPP


# include "../include/ourHeader.hpp"

class Poll
{
    private:
        std::vector<struct pollfd> socketsFd;

    public:
        // Poll(std::vector<int> socket_fd);
        Poll();
        ~Poll();
        bool    addSocket(int fd, short events);
        int waitForEvents(int timeout = -1);
        pollfd getEvent(int index);
        void removeSocket(int index);
        size_t getSocketsCount() const;


};


#endif