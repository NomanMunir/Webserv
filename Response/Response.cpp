/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmunir <nmunir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/03 15:22:38 by nmunir            #+#    #+#             */
/*   Updated: 2024/07/08 17:29:59 by nmunir           ###   ########.fr       */
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

std::string listDirectory(const std::string& dirPath, const std::string& uriPath)
{
    std::string htmlContent;

    DIR *dir;
    struct dirent *ent;
	std::string newUriPath = (uriPath == "/") ? "" : uriPath;

    if ((dir = opendir(dirPath.c_str())) != NULL) {
        htmlContent += "<html><head><title>Index of " + dirPath + "</title></head><body>";
        htmlContent += "<h1>Index of " + dirPath + "</h1><hr><ul>";

        while ((ent = readdir(dir)) != NULL)
		{
            std::string filename = ent->d_name;

            if (filename == "." || filename == "..")
                continue;
            htmlContent += "<li><a href=\"" + newUriPath + "/" + filename + "\">" + filename + "</a></li>";
        }

        htmlContent += "</ul><hr></body></html>";

        closedir(dir);
    } else {
        // Could not open directory
        std::cerr << "Error: Could not open directory " << dirPath << std::endl;
    }
    return htmlContent;
}

int Response::checkType(std::string &path, RouteConfig &targetRoute)
{
	struct stat info;
	if (stat(path.c_str(), &info) != 0)
		return (0);
	else if (S_ISDIR(info.st_mode))
		return (1);
	else if (S_ISREG(info.st_mode))
		return (2);
}

bool handleDirectory(std::string &fullPath, std::string &path, RouteConfig &targetRoute)
{
	for (size_t i = 0; i < targetRoute.defaultFile.size(); i++)
	{
		std::string newPath = path + "/" + targetRoute.defaultFile[i];
		if (isFile(newPath))
		{
			path = newPath;
			return (true);
		}
	}
	if (targetRoute.directoryListing)
	{
		std::string body = listDirectory(fullPath, path);
		response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " + std::to_string(body.size()) + "\r\n\r\n" + body;
	}
	else
		response404();
}
ServerConfig Response::chooseServer(Request &request, Parser &configFile)
{
	size_t count = 0;
	std::vector<ServerConfig>::iterator targetServer;
	std::string requestHost = request.getHeaders().getValue("Host");
	std::string requestPort = request.getHeaders().getValue("Port");
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

RouteConfig Response::chooseRoute(std::string path, ServerConfig &server)
{
	std::map<std::string, RouteConfig> routes = server.routeMap;
	RouteConfig targetRoute;
	std::map<std::string, RouteConfig>::iterator it = routes.begin();
	for (; it != routes.end(); it++)
	{
		if (path.find(it->first) == 0)
		{
			targetRoute = it->second;
			break;
		}
	}
		if (it == routes.end())
		{
			response404();
			throw std::runtime_error("Error: Route not found");
		}
	return targetRoute;
}

bool checkEndingSlash(std::string &fullPath, std::string &path)
{
	if (fullPath[fullPath.size() - 1] != '/')
		return false;
	return true;
}
void Response::handleGET(bool isGet, Request &request, Parser &configFile)
{
	if (isGet)
	{
		std::string path = request.getHeaders().getValue("uri");
		// std::cout << "RequestHost : " << requestHost << std::endl;
		// std::cout << "RequestUri : " << path << std::endl;
		ServerConfig targetServer = chooseServer(request, configFile);
		RouteConfig targetRoute = chooseRoute(path, targetServer);
		std::string fullPath = targetRoute.root + path;

		std::cout << "FullPath : " << fullPath << std::endl;
		int type = checkType(fullPath, targetRoute);
		if (type == 2)
		{
			std::ifstream file(fullPath.c_str());
			std::stringstream buffer;
			buffer << file.rdbuf();
			std::string body = buffer.str();
			response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " + std::to_string(body.size()) + "\r\n\r\n" + body;
		}
		else if (type == 1)
		{
			if (!checkEndingSlash(fullPath, path))
			{
				response = "HTTP/1.1 301 OK\r\nContent-Type: text/html\r\nContent-Length: 6 \r\n\r\n hello\n";
				std::cout << "end: " << fullPath[fullPath.size() - 1] << std::endl;
			}
			else
				handleDirectory(fullPath, path, targetRoute);
		}
		else if (type == 0)
			response404();
	}
}

void Response::handleResponse(Request &request, Parser &configFile)
{
	std::string method = request.getHeaders().getValue("method");
	std::string path = request.getHeaders().getValue("uri");
	std::string requestHost = request.getHeaders().getValue("Host");
	ServerConfig targetServer = chooseServer(request, configFile);
	RouteConfig targetRoute = chooseRoute(path, targetServer);
	if (!myFind(targetRoute.methods, method))
	{
		std::cout << "error 405 Method Not Allowed" << std::endl;
		response404();
		return;
	}
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
