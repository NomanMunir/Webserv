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
    serverAddr.sin_port = htons(8000);

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
void Server::printHeaders()
{
    std::cout << "Headers:\n";
    for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it)
    {
        std::cout << it->first << ": " << it->second << std::endl;
    }
}

void Server::handleRequest(int clientSocket)
{
    char buffer[1024];
    bzero(buffer, 1024);
    read(clientSocket, buffer, 1023);

    // parseFirstLine(buffer);
    storeHeaders(buffer);
    printHeaders();

    std::string response = generateHttpResponse(headers["uri"]);
    sendResponse(clientSocket, response);
}

std::string Server::generateHttpResponse(const std::string &filepath)
{
    std::ifstream file(std::string("." + filepath).c_str());
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

void Server::storeHeaders(const std::string &request)
{
    std::stringstream requestLine(request.c_str());
    // bool headersDone = false;
    std::getline(requestLine, startLine);
    // parseStartLine(startLine);
    storeFirstLine(startLine);
    std::string header;
    while (std::getline(requestLine, header)) {
        size_t colonPos = header.find(':');
        if (colonPos != std::string::npos) {
            std::string key = header.substr(0, colonPos);
            std::string value = header.substr(colonPos + 1);
            headers[key] = value;
        }
    }
}

void Server::storeFirstLine(const std::string &request)
{
    std::istringstream requestStream(request);
    std::string method;
    std::string uri;
    std::string version;

    requestStream >> method >> uri >> version;
    headers["method"] = method;
    headers["version"] = version;
    headers["uri"] = uri;
}