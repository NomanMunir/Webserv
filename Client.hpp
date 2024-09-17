#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <iostream>
#include <string>
#include "request/Request.hpp"
#include "response/Response.hpp"
#include "parsing/Parser.hpp"
#include "utils/utils.hpp"
#include "utils/Logs.hpp"
#include "cgi/Cgi.hpp"
#include "events/EventPoller.hpp"
#include "response/HttpResponse.hpp"
#include <time.h>

#define CLIENT_TIMEOUT 30

class Client {
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

    void handleNormalResponse();
    void handleCGI(ServerConfig &serverConfig);
    void validateCgiExtensions(std::vector<std::string> cgiExtensions, std::string fullPath);

    

public:
	Client();
    Client(int fd, EventPoller *poller);
	Client(const Client &c);
	Client& operator=(const Client &c);
    ~Client();

    int getFd() const;
    std::string& getWriteBuffer();
    Request& getRequest();
    Response& getResponse();
    Cgi& getCgi();
    bool isCGI() const;

    bool isWritePending() const;
    bool isReadPending() const;
    bool isKeepAlive();
    bool isTimeOut();

    void setWritePending(bool pending);
    void setReadPending(bool pending);
    void setKeepAlive(bool keepAlive);


    void updateLastActivity();
    void readFromSocket(ServerConfig &serverConfig);
    void reset();

};


#endif // CLIENT_HPP