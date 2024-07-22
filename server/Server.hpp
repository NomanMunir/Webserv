#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <vector>
#include <netinet/in.h>
#include <map>
#include <fcntl.h>
#include <poll.h>
#include "Poll.hpp"

class Poll;

class Server
{
public:
    Server();
    void run();
    std::map<std::string, std::string> headers;
    std::string startLine;
    std::string body;
    
    
private:
    int serverSocket;
    struct sockaddr_in serverAddr;
    // std::vector<struct pollfd> socketsFd;
    Poll pollManager;
    
    
    void initSocket();
    void handleConnections();
    void handleRequest(int clientSocket);
    void sendResponse(int clientSocket, const std::string &response);
    std::string generateHttpResponse(const std::string &filepath);
    void storeFirstLine(const std::string &request);
    void storeHeaders(const std::string &request);
    void printHeaders();
    void acceptNewConnection();
    // bool parseRequestLine(std::istringstream &stream);
    // bool parseHeaders(std::istringstream &stream);
    // void sendBadRequest();
    // void prepareAndSendResponse(int client_fd);
};

#endif // SERVER_HPP
