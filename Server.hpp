/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmunir <nmunir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/29 14:41:08 by nmunir            #+#    #+#             */
/*   Updated: 2024/07/20 17:45:12 by nmunir           ###   ########.fr       */
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
// #include <sys/epoll.h>
#include <sys/event.h>
#include <sys/time.h>

#include "utils/utils.hpp"
#include "parsing/Parser.hpp"
#include "parsing/Validation.hpp"
#include "request/Request.hpp"
#include "response/Response.hpp"

class Server
{
public:
    Server();
    void run(Parser &configFile);
    int getServersocket() const;

private:
    int serverSocket;
    struct sockaddr_in serverAddr;

    void initServerSocket();

    void handleConnections(Parser &configFile);
    void handleConnectionsWithSelect(Parser &configFile);
    // void handleConnectionsWithPoll(Parser &configFile);
    // void handleConnectionsWithEPoll(Parser &configFile);
    void handleConnectionsWithKQueue(Parser &configFile);
};

#endif // SERVER_HPP
