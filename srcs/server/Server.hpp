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

#include "../utils/utils.hpp"
#include "../parsing/Parser.hpp"
#include "../request/Request.hpp"
#include "../response/Response.hpp"
#include "../client/Client.hpp"
#include "../events/EventPoller.hpp"
#include "../utils/Logs.hpp"

#include <sys/wait.h>

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
        std::map<int, Client> &getClients();

        bool isMyClient(int fd);
        bool isMyCGI(int fd);
        bool acceptClient();

        void handleCgiRead(int clientFd);
        void handleRead(int fd);
        void handleWrite(int fd);
        void handleDisconnection(int fd);

        void checkTimeouts();

        int serverError;

    private:
        int serverSocket;
        struct sockaddr_in addr;
        int port;
        ServerConfig serverConfig;
        EventPoller *_poller;
        std::map<int, Client> clients;

        void initSocket();
        void socketInUse();
        void bindAndListen();
        void generateCGIError(Client &client, std::string errorCode);
};

#endif // SERVER_HPP
