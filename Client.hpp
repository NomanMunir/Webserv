#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <iostream>
#include <string>
#include "request/Request.hpp"
#include "response/Response.hpp"
#include "parsing/Parser.hpp"

class Client {
private:
    int fd;
    std::string readBuffer;
    std::string writeBuffer;
    bool writePending;
    bool readPending;
    bool keepAlive;

    Request request;
    Response response;

    void recvChunk();
    void recvHeader();
    void recvBody(); 
    void sendResponse();
public:
	Client();
    Client(int fd);
	Client(const Client &c);
	Client& operator=(const Client &c);
    ~Client();

    int getFd() const;
    std::string& getReadBuffer();
    std::string& getWriteBuffer();
    Request& getRequest();
    Response& getResponse();


    bool isWritePending() const;
    bool isReadPending() const;
    bool isKeepAlive();

    void setWritePending(bool pending);
    void setReadPending(bool pending);
    void setKeepAlive(bool keepAlive);

    void readFromSocket(Parser &configFile);
    void reset();


};


#endif // CLIENT_HPP