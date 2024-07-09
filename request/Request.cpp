/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmunir <nmunir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/02 12:31:21 by nmunir            #+#    #+#             */
/*   Updated: 2024/07/09 17:55:23 by nmunir           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"

ServerConfig chooseServer(Parser &configFile, std::string requestHost, std::string requestPort)
{
	size_t count = 0;
	std::vector<ServerConfig>::iterator targetServer;
	std::vector<ServerConfig> servers = configFile.getServers();
	std::vector<ServerConfig>::iterator it = servers.begin();

	for (; it != servers.end(); it++)
	{
		std::vector<std::vector<std::string> > ports = it->listen;

		for (size_t i = 0; i < ports.size(); i++)
		{
			if (ports[i][1] == requestPort)
				break;
			else
				continue;
		}
		std::vector<std::string> serverNames = it->serverName;
		if (myFind(serverNames, requestHost))
		{
			targetServer = it;
			count++;
		}
	}
	if (count != 1)
		return servers[0];
	return *targetServer;
}

void Request::handleRequest(int clientSocket, Parser &parser)
{
	std::string requestHeader;
    char buffer;
	std::string host;
	std::string port;
	ServerConfig targetServer;
	
	while (read(clientSocket, &buffer, 1) > 0)
    {
		if (!isascii(buffer))
			throw std::runtime_error("Non-ASCII character found in request header.");
        requestHeader.append(1, buffer);
        if (requestHeader.find("\r\n\r\n") != std::string::npos)
            break;
    }
	std::string::size_type hostPos = requestHeader.find("Host: ");
	if (hostPos != std::string::npos)
	{
	std::string::size_type endHostPos = requestHeader.find("\r\n", hostPos);	
	if (endHostPos != std::string::npos)
		host = requestHeader.substr(hostPos + 6, endHostPos - hostPos - 6);
		std::vector<std::string> tokens = split(host, ':');
		if (tokens.size() == 2)
		{
			if (!validateNumber("listen", tokens[1]))
			{
				targetServer = parser.getServers()[0];
				Response response("400", targetServer, parser);
			}
			else
				targetServer = chooseServer(parser, tokens[0], tokens[1]);
		}
		else
			targetServer = chooseServer(parser, tokens[0], "80");
	}
	
	headers = Headers(clientSocket);
	
	body = Body(clientSocket, headers, parser);
}

Request::Request(int clientSocket, Parser &parser)
{
	if (clientSocket == -1)
	{
		std::cerr << "Error accepting client connection." << std::endl;
		return;
	}
	handleRequest(clientSocket, parser);
}

Headers Request::getHeaders()
{
	return headers;
}

Body Request::getBody()
{
	return body;
}
Request::~Request() { }