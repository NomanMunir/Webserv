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

#include <string>
#include <vector>
#include <netinet/in.h>
#include <map>
#include <iostream>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <cerrno>
#include <dirent.h>
#include <poll.h>
#include <sys/time.h>

#include "utils/utils.hpp"
#include "parsing/Parser.hpp"
#include "parsing/Validation.hpp"
#include "request/Request.hpp"
#include "response/Response.hpp"

class Server
{
public:
    Server(ServerConfig &serverConfig);
    ~Server();
    Server(const Server &other);
    Server &operator=(const Server &other);

    void run();

    int getPort() const;
    int getServerSocket() const;
    struct sockaddr_in getAddr() const;
    ServerConfig getServerConfig() const;
    int serverError;

private:
    int serverSocket;
    struct sockaddr_in addr;
    int port;
    ServerConfig serverConfig;

    void bindAndListen();
    void socketInUse();
    void initSocket();
};

#endif // SERVER_HPP
