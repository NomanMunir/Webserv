/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmunir <nmunir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/03 15:22:38 by nmunir            #+#    #+#             */
/*   Updated: 2024/07/04 17:38:35 by nmunir           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"


void Response::response404()
{
	std::ifstream file("errors/404.html");
	std::string line;
	std::string body;
	while (std::getline(file, line))
	{
		body += line + "\n";
	}
	response = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\nContent-Length: " + std::to_string(body.size()) + "\r\n\r\n" + body;
}

void Response::checkType(std::string &path, RouteConfig &targetRoute)
{
	size_t i = 0;
	if (isDirectory(path))
	{
		for (; i < targetRoute.defaultFile.size(); i++)
		{
			std::string newPath = path + "/" + targetRoute.defaultFile[i];
			if (isFile(newPath))
			{
				path = newPath;
				break;
			}
		}
		if (i == targetRoute.defaultFile.size())
		{
			std::cout << "error 301" << std::endl;
			response404();
			return;
		}
	}
	else if (!isFile(path))
	{
		std::cout << "error 301" << std::endl;
		response404();
		return;
	}
	
}


void Response::handleGET(bool isGet, Request &request, Parser &configFile)
{
	if (isGet)
	{
		size_t i = 0;
		std::string path = request.getHeaders().getValue("uri");
		std::string requestHost = request.getHeaders().getValue("Host");
		std::cout << "RequestHost : " << requestHost << std::endl;
		std::cout << "RequestUri : " << path << std::endl;
		std::vector<ServerConfig> servers = configFile.getServers();
		RouteConfig targetRoute;

		for (; i < servers.size(); i++)
		{
			std::vector<std::string> serverNames = servers[i].serverName;
			if (std::find(serverNames.begin(), serverNames.end(), requestHost) != serverNames.end())
				break;
		}
		if (i == servers.size())
			i = 0;
		std::cout << "Index : " << i << std::endl;
		std::map<std::string, RouteConfig> routes = servers[i].routeMap;
		std::map<std::string, RouteConfig>::iterator it = routes.begin();
		for (; it != routes.end(); it++)
		{
			if (it->first == path)
			{
				targetRoute = it->second;
				break;
			}
		}
		if (it == routes.end())
		{
			response404();
			return;
		}
		std::cout << "targetRoute.root : " << targetRoute.root << std::endl;
		std::string fullPath = targetRoute.root + path;
			
		std::cout << "FullPath : " << fullPath << std::endl;
		checkType(fullPath, targetRoute);
		std::ifstream file(fullPath);
		std::string line;
		std::string body;
		while (std::getline(file, line))
		{
			body += line + "\n";
		}
		response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " + std::to_string(body.size()) + "\r\n\r\n" + body;
	}
}

void Response::handleResponse(Request &request, Parser &configFile)
{
	std::string method = request.getHeaders().getValue("method");
	handleGET(method == "GET", request, configFile);
	// handlePOST(method == "POST" ? true : false, request, configFile);
}

Response::Response(Request &request, Parser &configFile)
{
	handleResponse(request, configFile);
}

Response::~Response() { }

void Response::sendResponse(int clientSocket)
{
	write(clientSocket, response.c_str(), response.size());
}
std::string Response::getResponse()
{
	return response;
}

void Response::printResponse()
{
	std::cout << "Response: " << response << std::endl;
}
