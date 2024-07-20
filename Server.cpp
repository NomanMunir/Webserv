/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmunir <nmunir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/29 14:41:04 by nmunir            #+#    #+#             */
/*   Updated: 2024/07/20 17:39:02 by nmunir           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Connections.hpp"
#define PORT 8080

void setNonBlocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int Server::getServersocket() const
{
    return serverSocket;
}

Server::Server()
{
    initServerSocket();
}


void Server::initServerSocket() 
{
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        perror("Error opening socket");
        throw std::runtime_error("Error opening socket");
    }
    std::memset((char *)&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    int e = 1;
    if(setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&e, sizeof(e)) < 0) {
        perror("Error setsockopt option");
        close(serverSocket);
        throw std::runtime_error("Error setsockopt option");
    }
    
    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Error binding socket");
        throw std::runtime_error("Error binding socket");
    }

    if (listen(serverSocket, 3) < 0)
    {
        perror("Error listening on socket");
        throw std::runtime_error("Error listening on socket");
    }
    std::cout << "Server is running on port " << PORT << std::endl;
}

void Server::run(Parser &configFile)
{
    handleConnections(configFile);
}

void Server::handleConnections(Parser &configFile)
{
    Connections connections(serverSocket);
    connections.loop(configFile);
}


// void Server::handleConnectionsWithPoll(Parser &configFile)
// {

//     pollfd serverPollfd;
//     serverPollfd.fd = serverSocket;
//     serverPollfd.events = POLLIN;
//     pollfds.push_back(serverPollfd);
//     while (true)
//     {
//         int activity = poll(&pollfds[0], pollfds.size(), -1);
//         if (activity < 0 && errno != EINTR) {
//             perror("Error in poll");
//             exit(1);
//         }

//         if (pollfds[0].revents & POLLIN) {
//             int clientSocket = accept(serverSocket, nullptr, nullptr);
//             if (clientSocket < 0) {
//                 std::cerr << "Error accepting client connection" << std::endl;
//                 continue;
//             }
//             std::cout << "New connection, socket fd is " << clientSocket << std::endl;

//             pollfd clientPollfd;
//             clientPollfd.fd = clientSocket;
//             clientPollfd.events = POLLIN;
//             pollfds.push_back(clientPollfd);
//         }
//         for (size_t i = 1; i < pollfds.size(); ++i)
//         {
//             if (pollfds[i].revents & POLLIN)
//             {
//                 Response response(pollfds[i].fd);
//                 Request request(pollfds[i].fd, configFile, response);
//                 response.handleResponse(request);
//                 responseClient(pollfds[i].fd, response.getResponse());
//                 std::string keepAive = request.getHeaders().getValue("Connection");
//                 if (keepAive != "keep-alive")
//                 {
//                     if (close(pollfds[i].fd) == -1)
//                     {
//                         perror("Error closing client socket");
//                         exit(1);
//                     }
//                     pollfds.erase(pollfds.begin() + i);
//                 }
//             }
//         }
//     }
// }


void Server::handleConnectionsWithSelect(Parser &configFile)
{
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
            FD_SET(*it, &readSet);

        int activity = select(maxFd + 1, &readSet, NULL, NULL, NULL);

        if ((activity < 0) && (errno != EINTR)) {
            perror("Error in select");
            exit(1);
        }

        if (FD_ISSET(serverSocket, &readSet))
        {
            int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientLen);
            if (clientSocket < 0)
            {
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
                std::string keepAlive = request.getHeaders().getValue("Connection");

                response.handleResponse(request);
                responseClient(clientSocket, response.getResponse());
                if (keepAlive != "keep-alive")
                {
                    close(clientSocket);
                    clientSockets.erase(it);
                    break;
                }
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


void Server::handleConnectionsWithKQueue(Parser &configFile)
{
    struct sockaddr_in clientAddr;
    socklen_t clientLen = sizeof(clientAddr);

    int kqueueFd = kqueue();
    if (kqueueFd == -1)
    {
        perror("Error creating kqueue instance");
        exit(1);
    }

    struct kevent change;
    setNonBlocking(serverSocket);
    EV_SET(&change, serverSocket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);

    if (kevent(kqueueFd, &change, 1, NULL, 0, NULL) == -1)
    {
        perror("Error adding server socket to kqueue");
        close(kqueueFd);
        exit(1);
    }

    std::vector<struct kevent> events(MAX_EVENTS);

    while (true)
    {
        int nev = kevent(kqueueFd, NULL, 0, events.data(), events.size(), NULL);
        if (nev == -1)
        {
            perror("Error in kevent");
            close(kqueueFd);
            exit(1);
        }

        for (int i = 0; i < nev; ++i)
        {
            if (events[i].ident == serverSocket)
            {
                int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientLen);
                if (clientSocket < 0)
                {
                    std::cerr << "Error accepting client connection" << std::endl;
                    continue;
                }
                std::cout << "New connection, socket fd is " << clientSocket << ", ip is : " << inet_ntoa(clientAddr.sin_addr) << ", port : " << ntohs(clientAddr.sin_port) << std::endl;
                setNonBlocking(clientSocket);
                EV_SET(&change, clientSocket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
                if (kevent(kqueueFd, &change, 1, NULL, 0, NULL) == -1)
                {
                    perror("Error adding client socket to kqueue");
                    close(clientSocket);
                    continue;
                }
            }
            else
            {
                int clientSocket = events[i].ident;
                Response response(clientSocket);
                Request request(clientSocket, configFile, response);
                response.handleResponse(request);
                responseClient(clientSocket, response.getResponse());

                std::string keepAlive = request.getHeaders().getValue("Connection");
                if (keepAlive != "keep-alive")
                {
                    EV_SET(&change, clientSocket, EVFILT_READ, EV_DELETE, 0, 0, NULL);
                    if (kevent(kqueueFd, &change, 1, NULL, 0, NULL) == -1)
                    perror("Error removing client socket from kqueue");
                    if (close(clientSocket) == -1)
                        perror("Error closing client socket");    
                }
            }
        }
    }
}

// void Server::handleConnections(Parser &configFile)
// {
    
//     int kqueueFd = kqueue();
//     if (kqueueFd == -1)
//     {
//         perror("Error creating kqueue instance");
//         exit(1);
//     }

//     struct kevent change;
//     EV_SET(&change, serverSocket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);

//     if (kevent(kqueueFd, &change, 1, NULL, 0, NULL) == -1)
//     {
//         perror("Error adding server socket to kqueue");
//         close(kqueueFd);
//         exit(1);
//     }

//     std::vector<struct kevent> events(MAX_EVENTS);

//     while (true)
//     {
//         int nev = kevent(kqueueFd, NULL, 0, events.data(), events.size(), NULL);
//         if (nev == -1)
//         {
//             perror("Error in kevent");
//             close(kqueueFd);
//         }

//         for (int i = 0; i < nev; ++i)
//         {
//             if (events[i].ident == serverSocket)
//             {
//                 if (!addClientToQueue(change, kqueueFd))
//                     continue;
//             }
//             else if (events[i].filter == EVFILT_READ)
//             {
//                 if (!handleClient(events[i].ident, configFile, kqueueFd, change))
//                     continue;
//             }
//         }
//     }
// }


// void Server::handleConnections(Parser &configFile) {
//     struct sockaddr_in clientAddr;
//     socklen_t clientLen = sizeof(clientAddr);

//     int epollFd = epoll_create1(0);
//     if (epollFd == -1) {
//         perror("Error creating epoll instance");
//         exit(1);
//     }

//     struct epoll_event ev, events[MAX_EVENTS];
//     ev.events = EPOLLIN;
//     ev.data.fd = serverSocket;
//     if (epoll_ctl(epollFd, EPOLL_CTL_ADD, serverSocket, &ev) == -1) {
//         perror("Error adding server socket to epoll");
//         close(epollFd);
//         exit(1);
//     }

//     while (true)
//     {
//         int nfds = epoll_wait(epollFd, events, MAX_EVENTS, -1);
//         if (nfds == -1)
//         {
//             perror("Error in epoll_wait");
//             close(epollFd);
//             exit(1);
//         }

//         for (int i = 0; i < nfds; ++i)
//         {
//             if (events[i].data.fd == serverSocket)
//             {
//                 int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientLen);
//                 if (clientSocket < 0)
//                 {
//                     std::cerr << "Error accepting client connection" << std::endl;
//                     continue;
//                 }
//                 std::cout << "New connection, socket fd is " << clientSocket << std::endl;

//                 ev.events = EPOLLIN;
//                 ev.data.fd = clientSocket;
//                 if (epoll_ctl(epollFd, EPOLL_CTL_ADD, clientSocket, &ev) == -1)
//                 {
//                     perror("Error adding client socket to epoll");
//                     close(clientSocket);
//                     continue;
//                 }
//             } else
//             {
//                 int clientSocket = events[i].data.fd;
//                 Response response(clientSocket);
//                 Request request(clientSocket, configFile, response);
//                 response.handleResponse(request);
//                 responseClient(clientSocket, response.getResponse());

//                 if (response.keepAlive())
//                 {
//                     // Keep the connection open if keep-alive is enabled
//                     continue;
//                 }

//                 close(clientSocket);
//                 if (epoll_ctl(epollFd, EPOLL_CTL_DEL, clientSocket, nullptr) == -1) {
//                     perror("Error removing client socket from epoll");
//                 }
//             }
//         }
//     }
// }


