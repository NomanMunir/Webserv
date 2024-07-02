/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmunir <nmunir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/29 14:41:08 by nmunir            #+#    #+#             */
/*   Updated: 2024/07/02 12:43:58 by nmunir           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include "parsing/Parser.hpp"
#include "parsing/Validation.hpp"
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
#include <cerrno>
#include "headers/Headers.hpp"
#include "headers/Request.hpp"

class Parser;
class Headers;

class Server
{
public:
    Server(Parser &parser);
    void run();
    std::map<std::string, std::string> headers;
    std::string startLine;
    std::string body;

private:

    int serverSocket;
    struct sockaddr_in serverAddr;

    void initSocket();
    void handleConnections();
    void handleRequest(int clientSocket);
    void sendResponse(int clientSocket, const std::string &response);
    std::string generateHttpResponse(const std::string &filepath);
    void storeFirstLine(const std::string &request);
    void storeHeaders(const std::string &request);
    void printHeaders();

    // bool parseRequestLine(std::istringstream &stream);
    // bool parseHeaders(std::istringstream &stream);
    // void sendBadRequest();
};

#endif // SERVER_HPP
