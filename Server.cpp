/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abashir <abashir@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/29 14:41:04 by nmunir            #+#    #+#             */
/*   Updated: 2024/07/29 10:58:03 by abashir          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Connections.hpp"
// #include "ConnectionsPoll.hpp"
#define PORT 80

void setNonBlocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl");
        exit(EXIT_FAILURE);
    }
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl");
        exit(EXIT_FAILURE);
    }
}

std::vector<int> Server::getServersockets() const
{
    return this->serverSockets;
}

Server::Server(Parser &configFile) {
    initServerSockets(configFile);
}

void Server::initServerSockets(Parser &configFile) 
{
    std::vector<std::string> ports = configFile.getPorts(); // Assume this returns a list of ports to listen on

    for (std::vector<std::string>::iterator it = ports.begin(); it != ports.end(); ++it) {
        int port = std::atoi((*it).c_str());
        int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket < 0) {
            perror("Error opening socket");
            throw std::runtime_error("Error opening socket");
        }
        setNonBlocking(serverSocket);
        int e = 1;
        if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&e, sizeof(e)) < 0) {
            perror("Error setsockopt option");
            close(serverSocket);
            throw std::runtime_error("Error setsockopt option");
        }

        struct sockaddr_in serverAddr;
        std::memset((char *)&serverAddr, 0, sizeof(serverAddr));
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_port = htons(port);

        if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
            perror("Error binding socket");
            close(serverSocket);
            throw std::runtime_error("Error binding socket");
        }

        if (listen(serverSocket, 3) < 0) {
            perror("Error listening on socket");
            close(serverSocket);
            throw std::runtime_error("Error listening on socket");
        }

        serverSockets.push_back(serverSocket);
        serverAddrs.push_back(serverAddr);
        std::cout << "Server is running on port " << port << std::endl;
    }
}


void Server::run(Parser &configFile)
{
    handleConnections(configFile);
}

void Server::handleConnections(Parser &configFile)
{
    Connections connections(this->serverSockets, configFile);
    connections.loop();
}

