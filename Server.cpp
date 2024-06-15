#include "Server.hpp"
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fstream>
#include <sstream>

Server::Server()
{
    initSocket();
}

void Server::initSocket()
{
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0)
    {
        perror("Error opening socket");
        exit(1);
    }

    bzero((char *)&serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(8080);

    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        perror("Error binding socket");
        exit(1);
    }

    listen(serverSocket, 5);
}

void Server::run()
{
    handleConnections();
}

void Server::handleConnections()
{
    struct sockaddr_in clientAddr;
    socklen_t clientLen = sizeof(clientAddr);

    while (true)
    {
        int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientLen);
        if (clientSocket < 0)
        {
            perror("Error on accept");
            continue;
        }
        handleRequest(clientSocket);
        close(clientSocket);
    }
}

void Server::handleRequest(int clientSocket)
{
    char buffer[1024];
    bzero(buffer, 1024);
    read(clientSocket, buffer, 1023);

    std::cout << "Request received:\n"
              << buffer << std::endl;

    std::string uri = parseUri(buffer);
    std::cout << "Parsed URI: " << uri << std::endl;

    std::string response = generateHttpResponse(uri);
    sendResponse(clientSocket, response);
}

std::string Server::parseUri(const std::string &request)
{
    std::istringstream requestStream(request);
    std::string method;
    std::string uri;
    std::string version;

    requestStream >> method >> uri >> version;

    if (uri == "/")
    {
        uri = "/index.html";
    }

    return "." + uri; // Prepend "." to keep it within the current directory
}

std::string Server::generateHttpResponse(const std::string &filepath)
{
    std::ifstream file(filepath);
    if (!file.is_open())
    {
        std::cerr << "Could not open the file: " << filepath << std::endl;
        return "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\nFile not found";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    std::string content = buffer.str();
    std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " + std::to_string(content.size()) + "\r\n\r\n" + content;

    return response;
}

void Server::sendResponse(int clientSocket, const std::string &response)
{
    write(clientSocket, response.c_str(), response.length());
}
