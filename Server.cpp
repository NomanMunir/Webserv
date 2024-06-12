#include "Server.hpp"
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

Server::Server() {
    initSocket();
}

void Server::initSocket() {
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        perror("Error opening socket");
        exit(1);
    }

    bzero((char *)&serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(8080);

    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Error binding socket");
        exit(1);
    }

    listen(serverSocket, 5);
}

void Server::run() {
    handleConnections();
}

void Server::handleConnections() {
    struct sockaddr_in clientAddr;
    socklen_t clientLen = sizeof(clientAddr);

    while (true) {
        int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientLen);
        if (clientSocket < 0) {
            perror("Error on accept");
            continue;
        }
        handleRequest(clientSocket);
        close(clientSocket);
    }
}

void Server::handleRequest(int clientSocket) {
    char buffer[1024];
    bzero(buffer, 1024);
    read(clientSocket, buffer, 1023);

    std::cout << "Request received:\n" << buffer << std::endl;

    std::string response = "HTTP/1.1 200 OKContent-Type: text/plain\r\n\r\nHello, World!";
    sendResponse(clientSocket, response);
}

void Server::sendResponse(int clientSocket, const std::string& response) {
    write(clientSocket, response.c_str(), response.length());
}
