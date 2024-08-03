#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <iostream>
#include <string>
#include "request/Request.hpp"
#include "response/Response.hpp"

class Client {
private:
    int clientFd;
    std::string readBuffer;
    std::string writeBuffer;
    bool writePending;
    bool readPending;
    bool keepAlive;

    Request request;
    Response response;

public:
	Client();
    Client(int fd);
	Client(const Client &c);
	Client& operator=(const Client &c);
    ~Client();

    int getClientFd() const;
    std::string& getReadBuffer();
    std::string& getWriteBuffer();
    void reset();

    bool isWritePending() const;
    void setWritePending(bool pending);
    bool isReadPending() const;
    void setReadPending(bool pending);
    bool isKeepAlive();
    void setKeepAlive(bool keepAlive);

    Request& getRequest();
    Response& getResponse();

};


#endif // CLIENT_HPP