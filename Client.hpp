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

#define CLIENT_TIMEOUT 10

class Client {
private:
    int fd;
    std::string writeBuffer;
    bool writePending;
    bool readPending;
    bool keepAlive;
    char **env;
    time_t lastActivity;

    EventPoller *_poller;
    Request request;
    Response response;
    Cgi cgi;
    
    void recvChunk();
    void recvHeader();
    void recvBody();
    void handleNormalResponse(ServerConfig &serverConfig);
    void handleCGI(ServerConfig &serverConfig);
    

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