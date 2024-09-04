/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: absalem < absalem@student.42abudhabi.ae    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/29 14:41:08 by nmunir            #+#    #+#             */
/*   Updated: 2024/08/15 13:20:07 by absalem          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include "utils/utils.hpp"
#include "parsing/Parser.hpp"
#include "parsing/Validation.hpp"
#include "request/Request.hpp"
#include "response/Response.hpp"
#include "Client.hpp"
#include "events/EventPoller.hpp"
#include "utils/Logs.hpp"

class Server
{
public:
    Server(ServerConfig &serverConfig, EventPoller *poller);
    ~Server();
    Server(const Server &other);
    Server &operator=(const Server &other);

    void init();

    int getPort() const;
    int getServerSocket() const;
    struct sockaddr_in getAddr() const;
    ServerConfig getServerConfig() const;
    bool acceptClient();
    bool isMyClient(int fd);

    void handleRead(int fd);
    void handleWrite(int fd);
    void handleDisconnection(int fd);


    int serverError;

    std::unordered_map<int, Client> &getClients();
private:
    int serverSocket;
    struct sockaddr_in addr;
    int port;
    ServerConfig serverConfig;
    std::unordered_map<int, Client> clients;
    EventPoller *_poller;

    void bindAndListen();
    void socketInUse();
    void initSocket();

};

#endif // SERVER_HPP
