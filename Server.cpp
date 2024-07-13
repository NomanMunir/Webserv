/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmunir <nmunir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/29 14:41:04 by nmunir            #+#    #+#             */
/*   Updated: 2024/07/13 10:55:38 by nmunir           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::Server(Parser &parser)
{
    initSocket(parser);
}

void Server::initSocket(Parser &configFile) 
{
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        perror("Error opening socket");
        exit(1);
    }
    std::memset((char *)&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(8080);

    int e = 1;
    if(setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&e, sizeof(e)) < 0) {
        perror("Error setsockopt option");
        close(serverSocket);
        exit(1);
    }
    
    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Error binding socket");
        exit(1);
    }
    listen(serverSocket, 10);
}

void Server::run(Parser &configFile)
{
    handleConnections(configFile);
}

void Server::handleConnections(Parser &configFile)
{
    std::cout << "Server is running on port 8080" << std::endl;
    struct sockaddr_in clientAddr;
    socklen_t clientLen = sizeof(clientAddr);

    std::vector<int> clientSockets;
    clientSockets.push_back(serverSocket);

    while (true)
    {
        fd_set readSet;
        FD_ZERO(&readSet);

        int maxFd = *std::max_element(clientSockets.begin(), clientSockets.end());
        for (std::vector<int>::iterator it = clientSockets.begin(); it != clientSockets.end(); ++it)
        {
            FD_SET(*it, &readSet);
        }

        int activity = select(maxFd + 1, &readSet, NULL, NULL, NULL);

        if ((activity < 0) && (errno != EINTR)) {
            perror("Error in select");
            exit(1);
        }

        if (FD_ISSET(serverSocket, &readSet)) {
            int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientLen);
            if (clientSocket < 0) {
                std::cerr << "Error accepting client connection" << std::endl;
                continue;
            }
            std::cout << "New connection, socket fd is " << clientSocket << ", ip is : " << inet_ntoa(clientAddr.sin_addr) << ", port : " << ntohs(clientAddr.sin_port) << std::endl;

            clientSockets.push_back(clientSocket);
        }

         for (std::vector<int>::iterator it = clientSockets.begin() + 1; it != clientSockets.end(); ++it)
         {
            int clientSocket = *it;
            if (FD_ISSET(clientSocket, &readSet))
            {
                Response response(clientSocket);
                Request request(clientSocket, configFile, response);
                response.handleResponse(request);
                responseClient(clientSocket, response.getResponse());
                close(clientSocket);
                clientSockets.erase(it);
                break;
            }
        }
    }

    // struct sockaddr_in clientAddr;
    // socklen_t clientLen = sizeof(clientAddr);
    // while (true)
    // {
    //     int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientLen);
    //     if (clientSocket < 0)
    //     {
    //         std::cout <<"Error accepting client connection" << std::endl;
    //         exit(1);
    //     }
    //         Response response(clientSocket);
    //         Request request(clientSocket, configFile, response);
    //         response.handleResponse(request);
    //         responseClient(clientSocket, response.getResponse());
    //         if (close(clientSocket) == -1)
    //             throw std::runtime_error("Error closing client socket.");
    // }
}
