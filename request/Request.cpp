/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmunir <nmunir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/02 12:31:21 by nmunir            #+#    #+#             */
/*   Updated: 2024/07/14 17:48:00 by nmunir           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"
#include "../Response/Response.hpp"

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

std::string findMatch(std::string &path, std::map<std::string, RouteConfig> routes)
{
	std::map<std::string, RouteConfig>::iterator it = routes.begin();
	for (; it != routes.end(); it++)
	{
		std::string route =  trimChar(it->first, '/');
		if (path == route)
			return it->first;
	}
	if (it == routes.end())
		return "";
	return "";
}

bool chooseRoute(std::string path, ServerConfig server, RouteConfig &targetRoute)
{
	std::map<std::string, RouteConfig> routes = server.routeMap;
	std::vector<std::string> splitPath = split(path, '/');
	// std::cout << "splitPath size : " << splitPath.size() << std::endl;
	for (size_t i = 0; i < splitPath.size(); i++)
	{
		// std::cout << " i : " << i << " splitPath : " << splitPath[i] << std::endl;
		path = trimChar(path, '/');
		// std::cout << "path : " << path << std::endl;
		std::string route = findMatch(path, routes);
		if (route != "")
		{
			// std::cout << "what is route : " << route << std::endl;
			targetRoute = routes[route];
			return true;
		}
		path = path.substr(0, path.find_last_of('/'));
	}
	if (routes.find("/") != routes.end())
		return (targetRoute = routes["/"]), true;
	return false;
}

void Request::findServer(Response &structResponse, Parser &parser)
{
	std::string host;
	std::string requestHeader = this->headers.getRawHeaders();

	std::string::size_type hostPos = requestHeader.find("Host: ");
	std::cout << "requestHeader : " << requestHeader << std::endl;
	if (hostPos != std::string::npos)
	{
		std::string::size_type endHostPos = requestHeader.find("\r\n", hostPos);	
		std::cout << "hi2 \n";
		if (endHostPos != std::string::npos)
			host = requestHeader.substr(hostPos + 6, endHostPos - hostPos - 6);
			std::vector<std::string> tokens = split(host, ':');
			if (tokens.size() == 2)
			{
				if (!validateNumber("listen", tokens[1]))
				{
					structResponse.setTargetServer(parser.getServers()[0]);
					structResponse.sendError("400");
				}
				else
					structResponse.setTargetServer(chooseServer(parser, tokens[0], tokens[1]));
			}
			else
				structResponse.setTargetServer(chooseServer(parser, tokens[0], "80"));
	}
	else
	{
		std::cout << "hi3 \n";
		structResponse.setTargetServer(parser.getServers()[0]);
		structResponse.sendError("400");
	}
	
}

bool Request::isBodyExistRequest(Parser &parser, Response &structResponse)
{
	
	std::string length = headers.getValue("Content-Length");
	std::string encoding = headers.getValue("Transfer-Encoding");
	if (length.empty() && encoding.empty())
		return false;
	if (!length.empty())
	{
		if (length.empty() || !validateNumber("Content-Length", length))
			structResponse.sendError("411");
		if (std::atof(length.c_str()) > std::atof(parser.getDirectives()["client_body_size"].c_str()))
			structResponse.sendError("413");
	}
	if (!encoding.empty())
	{
		if (encoding != "chunked")
			structResponse.sendError("411");
	}
	return true;
}

void Request::handleRequest(int clientSocket, Parser &parser, Response &structResponse)
{
	this->headers = Headers(clientSocket, structResponse);

	this->findServer(structResponse, parser);
	
	this->headers.parseHeader(structResponse);
	ServerConfig server = structResponse.getTargetServer();
	RouteConfig route;
	if (!chooseRoute(headers.getValue("uri"), server, route))
		structResponse.sendError("404");
	structResponse.setTargetRoute(route);
	if (isBodyExistRequest(parser, structResponse))
		this->body = Body(clientSocket, headers.getValue("Content-Length"));
}

Request::Request(int clientSocket, Parser &parser, Response &structResponse)
{
	if (clientSocket == -1)
	{
		std::cerr << "Error accepting client connection." << std::endl;
		return;
	}
	handleRequest(clientSocket, parser, structResponse);
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