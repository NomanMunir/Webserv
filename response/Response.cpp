/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abashir <abashir@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/03 15:22:38 by nmunir            #+#    #+#             */
/*   Updated: 2024/07/29 11:33:37 by abashir          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"

std::string Response::listDirectory(const std::string& dirPath, const std::string& uriPath)
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
            htmlContent += "<li><a href=\"" + newUriPath  + filename + "\">" + filename + "</a></li>";
        }

        htmlContent += "</ul><hr></body></html>";

        closedir(dir);
    } else {
        // Could not open directory
        std::cerr << "Error: Could not open directory " << dirPath << std::endl;
		this->sendError("404");
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
            this->sendError("403");
			break;
		}
        case ENOENT:
		{
			// std::cout << "Error: " << "Check Type" << std::endl;
            this->sendError("404");
			break;
		}
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

std::string resolvePath(std::string &fullPath, std::string &defaultFile)
{
	std::vector<std::string> rootTokens = split(fullPath, '/');
    std::vector<std::string> pathTokens = split(defaultFile, '/');

    std::vector<std::string> resolvedTokens = rootTokens;
	std::cout << "pathtokens: " << pathTokens.size() << std::endl;
    for (size_t i = 0; i < pathTokens.size(); ++i)
    {
        if (pathTokens[i] == "..")
        {
            if (!resolvedTokens.empty())
                resolvedTokens.pop_back();
        }
        else if (pathTokens[i] != ".")
            resolvedTokens.push_back(pathTokens[i]);
    }

    return (join(resolvedTokens, '/'));
}
bool Response::handleDirectory(std::string &fullPath, std::string &path, RouteConfig &targetRoute)
{
	std::string newPath;
	for (size_t i = 0; i < targetRoute.defaultFile.size(); i++)
	{
		if (targetRoute.defaultFile[i][0] == '/')
			newPath = targetRoute.defaultFile[i];
		else
			newPath = resolvePath(fullPath, targetRoute.defaultFile[i]);
		std::cout << "New Path: " << newPath << std::endl;
		// std::string newPath = fullPath + targetRoute.defaultFile[i];
		if (newPath.find(targetRoute.root) != 0)
		{
			std::cout << "Error: " << "What are you trying to access oui?" << std::endl;
			this->sendError("403");
			return (false);
		}
		if (isFile(newPath))
		{
			fullPath = newPath;
			return (true);
		}
	}
	if (targetRoute.directoryListing)
	{
		std::string body = listDirectory(fullPath, path);
		response = "HTTP/1.1 200 OK\r\nConnection: keep-alive\r\nContent-Type: text/html\r\nContent-Length: " + std::to_string(body.size()) + "\r\n\r\n" + body;
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

std::string generateFullPath(std::string rootPath, std::string path)
{
	if (rootPath.back() == '/')
		rootPath.pop_back();
	if (path.front() == '/')
		path.erase(0, 1);
	std::string fullPath = rootPath + "/" + path;
	return fullPath;
}

void Response::generateResponseFromFile(std::string &path)
{
	std::ifstream file(path.c_str());
	std::stringstream buffer;
	buffer << file.rdbuf();
	std::string body = buffer.str();
	std::string extention = path.substr(path.find_last_of(".") + 1);
	std::string mimeType = getMimeType(extention);
	response = "HTTP/1.1 200 OK\r\nConnection: keep-alive\r\nContent-Type: " + mimeType + "\r\nContent-Length: " + std::to_string(body.size()) + "\r\n\r\n" + body;
}

void Response::handleRedirect(std::string redirect)
{
	std::cout << "Redirect: " << redirect << std::endl;
	std::stringstream ss(redirect);
	int errorCode;
	ss >> errorCode;
	std::string value;
	std::getline(ss, value, '\0');

	removeCharsFromString(value, "\"'");

	if (!value.empty())
	{
		if (errorCode >= 300 && errorCode < 400)
		{
			value =  trim(value);
			std::string tokenWithSlash = value[0] == '/' ? value :  "/" + value;
			tokenWithSlash = trim(tokenWithSlash);
			response = "HTTP/1.1 " + std::to_string(errorCode) + " " + getErrorMsg(std::to_string(errorCode)) + "\r\nLocation: " + tokenWithSlash + "\r\n\r\n";
		}
		else
			response = "HTTP/1.1" + std::to_string(errorCode) + " " + getErrorMsg(std::to_string(errorCode)) + "\r\nContent-Type: text/html\r\nContent-Length: " + std::to_string(value.size()) + "\r\n\r\n" + value + "\n";
	}
	else
		sendError(std::to_string(errorCode));
}

void Response::handleGET(bool isGet, RouteConfig &targetRoute, std::string &path)
{
	std::cout << "Path : " << path << std::endl;
	if (isGet)
	{
		std::string fullPath = generateFullPath(targetRoute.root, path);
		if (targetRoute.redirect != "")
		{
			handleRedirect(targetRoute.redirect);
			return;
		}

		std::cout << "FullPath : " << fullPath << std::endl;
		int type = checkType(fullPath, targetRoute);
		if (type == 2)
			generateResponseFromFile(fullPath);
		else if (type == 1)
		{
			if (!checkEndingSlash(fullPath))
				this->sendError("301");
			else
			{
				if(handleDirectory(fullPath, path, targetRoute))
					generateResponseFromFile(fullPath);
			}
		}
	}
}

void Response::handlePOST(bool isPost, RouteConfig &targetRoute, std::string &path, Body &body)
{
	if (isPost)
	{
		std::cout << "Post" << std::endl;
		// body.printBody();
		// std::string fullPath = targetRoute.root + path;
		// int type = checkType(fullPath, targetRoute);
		// if (type == 2)
		// {
		// 	std::ofstream file(fullPath.c_str());
		// 	file << body.getBody();
		// 	file.close();
		// 	response = "HTTP/1.1 200 OK\r\nConnection: keep-alive\r\nContent-Type: text/html\r\nContent-Length: 6 \r\n\r\n hello\n";
		// }
		// else if (type == 1)
		// {
		// 	sendError("403");
		// }
		// else if (type == 0)
		// 	sendError("403");
	}
}

void Response::handleResponse(Request &request)
{
	std::string method = request.getHeaders().getValue("method");
	std::string uri = request.getHeaders().getValue("uri");
	Body body = request.getBody();

	if (!myFind(this->targetRoute.methods, method))
		sendError("403");

	handleGET(method == "GET", this->targetRoute, uri);

	handlePOST(method == "POST", this->targetRoute, uri, body);
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
			   "Connection: keep-alive\r\n"
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
	if (myFind(closeCodes, errorCode))
		isConClosed = true;
}


bool Response::getIsConClosed()
{
	return isConClosed;
}

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


Response::Response() : isConClosed(false) { }

Response::Response(const Response &c) : statusCodes(c.statusCodes), response(c.response), targetServer(c.targetServer), targetRoute(c.targetRoute), isConClosed(c.isConClosed) { }

Response& Response::operator=(const Response &c)
{
	if (this == &c)
		return *this;
	statusCodes = c.statusCodes;
	response = c.response;
	targetServer = c.targetServer;
	targetRoute = c.targetRoute;
	isConClosed = c.isConClosed;
	return *this;
}

void Response::reset()
{
	response = "";
}