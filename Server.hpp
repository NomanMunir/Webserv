#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <vector>
#include <netinet/in.h>

class Server
{
public:
    Server();
    void run();

private:
    int serverSocket;
    struct sockaddr_in serverAddr;

    void initSocket();
    void handleConnections();
    void handleRequest(int clientSocket);
    void sendResponse(int clientSocket, const std::string &response);
    std::string generateHttpResponse(const std::string &filepath);
    std::string parseUri(const std::string &request);
};

#endif // SERVER_HPP
