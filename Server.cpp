/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmunir <nmunir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/29 14:41:04 by nmunir            #+#    #+#             */
/*   Updated: 2024/07/02 13:17:57 by nmunir           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::Server(Parser &parser)
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
    const char *ipAddress = "127.0.0.1";
    serverAddr.sin_addr.s_addr = inet_addr(ipAddress);
    serverAddr.sin_port = htons(8080);

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
        Request(accept(serverSocket, (struct sockaddr *)&clientAddr, &clientLen));
        
    }
}

std::string Server::generateHttpResponse(const std::string &filepath)
{
    std::string path = filepath;
    if (filepath == "/")
        path = "." + filepath + "index.html";
    std::ifstream file(path.c_str());
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
    std::stringstream requestStream(request.c_str());
    // bool headersDone = false;
    std::getline(requestStream, startLine);
    storeFirstLine(startLine);
    std::string header;
    while (std::getline(requestStream, header))
    {
        if (header == "\r" || header.empty())
            break;
        size_t colonPos = header.find(':');
        if (colonPos != std::string::npos)
        {
            std::string key = header.substr(0, colonPos);
            std::string value = header.substr(colonPos + 1);
            size_t start = value.find_first_not_of(" \t");
            if (start != std::string::npos)
                value = value.substr(start);
            headers[key] = value;
        }
    }
    // Read the body
    std::map<std::string, std::string>::iterator it = headers.find("Content-Length");
    if (it != headers.end())
    {
        size_t contentLength = std::atoi(it->second.c_str());
        body.resize(contentLength);
        requestStream.read(&body[0], contentLength);
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
