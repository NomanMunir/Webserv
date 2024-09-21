#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "../request/Request.hpp"
#include "../response/Response.hpp"
#include "../parsing/Parser.hpp"
#include "../utils/utils.hpp"
#include "../utils/Logs.hpp"
#include "../cgi/Cgi.hpp"
#include "../events/EventPoller.hpp"
#include "../response/HttpResponse.hpp"

#include <iostream>
#include <string>
#include <time.h>

#define CLIENT_TIMEOUT 600
#define REQUEST_TIMEOUT 500
class Client 
{
    private:
        EventPoller *_poller;
        Request request;
        Response response;
        Cgi cgi;

        int fd;
        time_t lastActivity;

        std::string writeBuffer;
        bool writePending;
        bool readPending;
        bool keepAlive;
        char **env;

        void recvChunk();
        void recvHeader();
        void recvBody();

        void handleCGI(ServerConfig &serverConfig);
        void handleNormalResponse();
        void validateCgiExtensions(std::vector<std::string> cgiExtensions, std::string fullPath);

    public:
        Client();
        Client(int fd, EventPoller *poller);
        Client(const Client &c);
        Client& operator=(const Client &c);
        ~Client();

        int getFd() const;
        Cgi& getCgi();
        bool isCGI() const;
        Request& getRequest();
        Response& getResponse();
        std::string& getWriteBuffer();

        bool isTimeOut(int timeout);
        bool isKeepAlive();
        bool isReadPending() const;
        bool isWritePending() const;

        void setKeepAlive(bool keepAlive);
        void setReadPending(bool pending);
        void setWritePending(bool pending);

        void updateLastActivity();
        void readFromSocket(ServerConfig &serverConfig);
        void reset();
};

#endif // CLIENT_HPP