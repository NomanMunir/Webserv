/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmunir <nmunir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/02 12:31:21 by nmunir            #+#    #+#             */
/*   Updated: 2024/07/02 14:51:04 by nmunir           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"

void sendResponse(int clientSocket, const std::string &response)
{
    write(clientSocket, response.c_str(), response.length());
}

void handleRequest(int clientSocket)
{
    std::string request;
    char buffer;
    ssize_t bytesRead;
    
    while (read(clientSocket, &buffer, 1) > 0)
    {
		if (!isascii(buffer))
			throw std::runtime_error("Non-ASCII character found in request header.");
        request.append(1, buffer);
        if (request.find("\r\n\r\n") != std::string::npos)
            break;
    }
    Headers header(request);
    // size_t contentLengthPos = request.find("Content-Length:");
    // if (contentLengthPos != std::string::npos)
    // {
    //     size_t contentLengthEnd = request.find("\r\n", contentLengthPos);
    //     std::string contentLengthStr = request.substr(contentLengthPos + 15, contentLengthEnd - (contentLengthPos + 15));
    //     double contentLength = std::atof(contentLengthStr.c_str());
	// 	// Parsing the content length
    //     std::size_t bodyPos = request.find("\r\n\r\n") + 4;
    //     int bodyLength = request.size() - bodyPos;

    //     while (bodyLength < contentLength)
    //     {
    //         bytesRead = read(clientSocket, &buffer, sizeof(buffer));
    //         request.append(buffer, bytesRead);
    //         bodyLength += bytesRead;
    //     }
    // }
    // printHeaders();
    // std::string response = generateHttpResponse(headers["uri"]);
	std::string content = "Hello, World!";
    std::string response = "HTTP/1.1 200 OK\rContent-Type: text/html\r\nContent-Length: " + std::to_string(content.size()) + "\r\n\r\n" + content;
    sendResponse(clientSocket, response);
}

Request::Request(int clientSocket)
{
	if (clientSocket == -1)
	{
		std::cerr << "Error accepting client connection." << std::endl;
		return;
	}
	handleRequest(clientSocket);
	if (close(clientSocket) == -1)
		throw std::runtime_error("Error closing client socket.");
	
}

Request::~Request() { }