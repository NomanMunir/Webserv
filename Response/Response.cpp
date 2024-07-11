/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmunir <nmunir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/03 15:22:38 by nmunir            #+#    #+#             */
/*   Updated: 2024/07/11 17:04:30 by nmunir           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"

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
	{
		switch (errno)
        {
        case EACCES:
		{
			std::cout << "Error: " << "Eacces" << std::endl;
            this->sendError("403");
		}
        case ENOENT:
            this->sendError("404");
        default:
			this->sendError("500");
        }
	}
	if (S_ISREG(info.st_mode))
		return (2);
	else if (S_ISDIR(info.st_mode))
		return (1);
	return (0);
}

bool Response::handleDirectory(std::string &fullPath, std::string &path, RouteConfig &targetRoute)
{
	for (size_t i = 0; i < targetRoute.defaultFile.size(); i++)
	{
		std::string newPath = fullPath + targetRoute.defaultFile[i];
		std::cout << "NewPath : " << newPath << std::endl;
		if (isFile(newPath))
		{
			fullPath = newPath;
			return (true);
		}
	}
	if (targetRoute.directoryListing)
	{
		std::string body = listDirectory(fullPath, path);
		response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " + std::to_string(body.size()) + "\r\n\r\n" + body;
	}
	else
	{
		std::cout << "Error: " << "Handle Directory" << std::endl;
		
		this->sendError("403");
	}
	return (false);
}

bool checkEndingSlash(std::string &fullPath)
{
	if (fullPath[fullPath.size() - 1] != '/')
		return false;
	return true;
}

std::string makeFullPath(std::string rootPath, std::string path)
{
	if (rootPath.back() == '/')  
		rootPath.pop_back();
	if (path.front() == '/')
		path.erase(0, 1);
	std::string fullPath = rootPath + "/" + path;
	return fullPath;
}

void Response::handleGET(bool isGet, RouteConfig &targetRoute, std::string &path)
{
	if (isGet)
	{
		std::string fullPath = makeFullPath(targetRoute.root, path);
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
			if (!checkEndingSlash(fullPath))
				this->sendError("301");
			else
			{
				if(handleDirectory(fullPath, path, targetRoute))
				{
					std::ifstream file(fullPath.c_str());
					std::stringstream buffer;
					buffer << file.rdbuf();
					std::string body = buffer.str();
					response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " + std::to_string(body.size()) + "\r\n\r\n" + body;
				}
				
			}
		}
	}
}

void Response::handlePOST(bool isPost, RouteConfig &targetRoute, std::string &path, Body &body)
{
	if (isPost)
	{
		std::string fullPath = targetRoute.root + path;
		int type = checkType(fullPath, targetRoute);
		if (type == 2)
		{
			std::ofstream file(fullPath.c_str());
			file << body.getBody();
			file.close();
			response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: 6 \r\n\r\n hello\n";
		}
		// else if (type == 1)
		// {
		// 	response404();
		// }
		// else if (type == 0)
		// 	response404();
	}
}

void Response::handleResponse(Request &request)
{
	std::string method = request.getHeaders().getValue("method");
	std::string uri = request.getHeaders().getValue("uri");

	if (!myFind(this->targetRoute.methods, method))
		sendError("403");
		
	handleGET(method == "GET", this->targetRoute, uri);
	// handlePOST(method == "POST", targetRoute, uri, body);
}

void Response::defaultErrorPage(std::string errorCode)
{
    std::string body = "<html><head><title>Error " + errorCode + " " + getErrorMsg(errorCode) + "</title></head>"
                       "<body style='background-color:lime; color:purple; font-family:Comic Sans MS;'>"
                       "<center><h1 style='font-size:50px; border:5px dotted red;'>Oops! Error " + errorCode + " " + getErrorMsg(errorCode) +"</h1></center>"
                       "<center><p style='font-size:30px; border:3px dashed orange;'>Something went terribly wrong!</p></center>"
                       "<marquee behavior='scroll' direction='left' style='font-size:20px; color:blue;'>This is an ugly error page!</marquee>"
                       "</body></html>";
    response = "HTTP/1.1 " + errorCode + " " + getErrorMsg(errorCode) + "\r\n"
               "Content-Type: text/html\r\n"
               "Content-Length: " + std::to_string(body.size()) + "\r\n\r\n"
               + body;
}


void Response::findErrorPage(std::string errorCode, std::map<std::string, std::string> errorPages)
{
	std::map<std::string, std::string>::iterator it = errorPages.find(errorCode);
	if (it != errorPages.end())
	{
		std::ifstream file("." + it->second);
		std::stringstream buffer;
		buffer << file.rdbuf();
		std::string body = buffer.str();
		response = "HTTP/1.1 " + errorCode + " " + getErrorMsg(errorCode) + "\r\nContent-Type: text/html\r\nContent-Length: " + std::to_string(body.size()) + "\r\n\r\n" + body;
	}
	else
		defaultErrorPage(errorCode);
}

void Response::sendError(std::string errorCode)
{
	std::vector<std::string> closeCodes;
	closeCodes.push_back("400");
	closeCodes.push_back("403");
	closeCodes.push_back("404");
	closeCodes.push_back("405");
	closeCodes.push_back("413");
	closeCodes.push_back("414");
	closeCodes.push_back("500");
	closeCodes.push_back("501");

	std::map<std::string, std::string> errorPages = targetServer.errorPages;
	findErrorPage(errorCode, errorPages);
	responseClient(this->clientSocket, this->response);
	if (myFind(closeCodes, errorCode))
	{
		// close(this->clientSocket);
		
		// throw std::runtime_error("Error: " + errorCode);
	}
}


Response::Response(int clientFd) : clientSocket(clientFd) { }

Response::~Response() { }

std::string Response::getResponse()
{
	return (response);
}

void Response::setTargetServer(ServerConfig server)
{
	targetServer = server;
}

void Response::setTargetRoute(RouteConfig route)
{
	targetRoute = route;
}

ServerConfig Response::getTargetServer()
{
	return targetServer;
}

RouteConfig Response::getTargetRoute()
{
	return targetRoute;
}

void Response::printResponse()
{
	std::cout << "Response: " << response << std::endl;
}
