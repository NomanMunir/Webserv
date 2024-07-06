/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmunir <nmunir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/03 15:22:38 by nmunir            #+#    #+#             */
/*   Updated: 2024/07/06 16:50:51 by nmunir           ###   ########.fr       */
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

std::string listDirectory(const std::string& dirPath)
{
    std::string htmlContent;

    // Open the directory
    DIR *dir;
    struct dirent *ent;

    if ((dir = opendir(dirPath.c_str())) != NULL) {
        // Start building HTML content
        htmlContent += "<html><head><title>Index of " + dirPath + "</title></head><body>";
        htmlContent += "<h1>Index of " + dirPath + "</h1><hr><ul>";

        while ((ent = readdir(dir)) != NULL) {
            std::string filename = ent->d_name;

            // Skip "." and ".." entries
            if (filename == "." || filename == "..")
                continue;

            // Add a list item with a hyperlink to each file
            htmlContent += "<li><a href=\"" + filename + "\">" + filename + "</a></li>";
        }

        htmlContent += "</ul><hr></body></html>";

        closedir(dir);
    } else {
        // Could not open directory
        std::cerr << "Error: Could not open directory " << dirPath << std::endl;
    }
    return htmlContent;
}

bool Response::checkType(std::string &path, RouteConfig &targetRoute)
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
			// std::cout << "error 301" << std::endl;
			// response404();
			return false;
		}
	}
	else if (!isFile(path))
	{
		std::cout << "error 301" << std::endl;
		response404();
		return false;
	}
	return true;
}

void Response::handleGET(bool isGet, Request &request, Parser &configFile)
{
	if (isGet)
	{
		size_t i = 0;
		std::string path = request.getHeaders().getValue("uri");
		std::string requestHost = request.getHeaders().getValue("Host");
		// std::cout << "RequestHost : " << requestHost << std::endl;
		// std::cout << "RequestUri : " << path << std::endl;
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
		// std::cout << "targetRoute.root : " << targetRoute.root << std::endl;
		std::string fullPath = targetRoute.root + path;

		std::cout << "FullPath : " << fullPath << std::endl;
		if (checkType(fullPath, targetRoute))
		{
			std::ifstream file(fullPath.c_str());
			std::stringstream buffer;
			buffer << file.rdbuf();
			std::string body = buffer.str();
			response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " + std::to_string(body.size()) + "\r\n\r\n" + body;
		}
		else
		{
			if (targetRoute.directoryListing)
			{
				std::string body = listDirectory(fullPath);
				response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " + std::to_string(body.size()) + "\r\n\r\n" + body;
			}
			else
				response404();
		}
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
