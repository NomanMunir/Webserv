/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmunir <nmunir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/02 12:31:21 by nmunir            #+#    #+#             */
/*   Updated: 2024/07/03 17:12:12 by nmunir           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"

void sendResponse(int clientSocket, const std::string &response)
{
    write(clientSocket, response.c_str(), response.length());
}

void Request::handleRequest(int clientSocket)
{   
    // headers = Headers(clientSocket);
    // body = Body(clientSocket, headers);
    
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