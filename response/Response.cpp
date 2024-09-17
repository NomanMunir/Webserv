/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: absalem < absalem@student.42abudhabi.ae    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/03 15:22:38 by nmunir            #+#    #+#             */
/*   Updated: 2024/08/18 13:43:39 by absalem          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"
#include "../cgi/Cgi.hpp"
#include "../utils/MimeTypes.hpp"

int Response::checkType(std::string path)
{
	struct stat info;
	if (stat(path.c_str(), &info) != 0)
	{
		switch (errno)
        {
        case EACCES:
		{
            this->setErrorCode(403, "[checkType]\t\t Access denied " + path); break;
		}
        case ENOENT:
		{
            this->setErrorCode(404, "[checkType]\t\t File not found " + path); break;
		}
        default:
			this->setErrorCode(500, "[checkType]\t\t Could not get file info " + path);
        }
	}
	if (S_ISREG(info.st_mode))
		return (IS_FILE);
	else if (S_ISDIR(info.st_mode))
		return (IS_DIR);
	return (0);
}

std::string Response::generateDirectoryListing(const std::string& dirPath, const std::string& uriPath)
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
			if (isFileDir(dirPath + filename) == IS_DIR)
				filename += "/";
            htmlContent += "<li><a href=\"" + newUriPath  + filename + "\">" + filename + "</a></li>";
        }

        htmlContent += "</ul><hr></body></html>";
        closedir(dir);
    }
	else
		this->setErrorCode(404, "[generateDirectoryListing]\t\t Could not open directory");
    return htmlContent;
}

std::string resolvePath(std::string &fullPath, std::string &defaultFile)
{
	std::vector<std::string> rootTokens = split(fullPath, '/');
    std::vector<std::string> pathTokens = split(defaultFile, '/');

    std::vector<std::string> resolvedTokens = rootTokens;
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

bool Response::checkDefaultFile(std::string &fullPath, bool isCGI)
{
	std::string newPath;
	std::vector <std::string> defaultFiles = targetRoute.defaultFile;
	std::string root = targetRoute.root;
	if (isCGI)
	{
		defaultFiles = targetServer.defaultFile;
		root = targetServer.root;
	}
	for (size_t i = 0; i < defaultFiles.size(); i++)
	{
		if (defaultFiles[i][0] == '/')
			newPath = defaultFiles[i];
		else
			newPath = resolvePath(fullPath, defaultFiles[i]);
		if (newPath.find(root) != 0)
		{
			Logs::appendLog("Error", "What are you trying to access oui? " + newPath);
			this->setErrorCode(403, "[checkDefaultFile]\t\t Access denied");
		}
		if (isFileDir(newPath) == IS_FILE && isCGI)
		{
			std::string fileExtension = newPath.substr(newPath.find_last_of("."));
			if (targetServer.cgiExtensions.size() == 1 && targetServer.cgiExtensions[0] == "")
				continue;
			if (std::find(targetServer.cgiExtensions.begin(), targetServer.cgiExtensions.end(), fileExtension) == targetServer.cgiExtensions.end())
				continue;
			fullPath = newPath;
			return (true);
		}
		else if (isFileDir(newPath) == IS_FILE)
			return (generateResponseFromFile(newPath, false), true);
	}
	return (false);
}

void Response::handleDirectory(std::string &fullPath, std::string &uri, bool &isCGI)
{
	if (fullPath.empty())
		this->setErrorCode(404, "[handleDirectory]\t\t Directory not found");
	if (fullPath[fullPath.size() - 1] != '/')
		this->setErrorCode(301, "[handleDirectory]\t\t Redirecting to directory with trailing slash");
	if(checkDefaultFile(fullPath, isCGI))
		return ;
	if (isCGI || this->targetRoute.directoryListing)
	{
		std::string body = generateDirectoryListing(fullPath, uri);
		HttpResponse httpResponse;
		httpResponse.setVersion("HTTP/1.1");
		httpResponse.setStatusCode(200);
		httpResponse.setHeader("Content-Type", "text/html");
		httpResponse.setHeader("Content-Length", intToString(body.size()));
		httpResponse.setHeader("Connection", "keep-alive");
		if (this->cookies != "")
			httpResponse.setHeader("Set-Cookie", this->cookies);
		httpResponse.setHeader("Server", "LULUGINX");
		httpResponse.setBody(body);
		response = httpResponse.generateResponse();
		isCGI = false;
	}
	else
		this->setErrorCode(403, "[handleDirectory]\t\t Directory listing is disabled");
}

void Response::generateResponseFromFile(std::string &path, bool isHEAD)
{
	std::ifstream file(path.c_str());
	std::stringstream buffer;
	buffer << file.rdbuf();
	std::string body = buffer.str();
	std::string extention = path.substr(path.find_last_of(".") + 1);
	std::string mimeType = MimeTypes::getType(extention);
	HttpResponse httpResponse;
	httpResponse.setVersion("HTTP/1.1");
	httpResponse.setStatusCode(200);
	httpResponse.setHeader("Content-Type", mimeType);
	httpResponse.setHeader("Content-Length", intToString(body.size()));
	httpResponse.setHeader("Connection", "keep-alive");
	if (this->cookies != "")
		httpResponse.setHeader("Set-Cookie", this->cookies);
	httpResponse.setHeader("Server", "LULUGINX");
	if (!isHEAD)
		httpResponse.setBody(body);
	response = httpResponse.generateResponse();
}

bool Response::handleRedirect(bool isRedir, std::string redirect)
{
	if (!isRedir)
		return false;
	std::stringstream ss(redirect);
	int errorCode;
	ss >> errorCode;
	std::string value;
	std::getline(ss, value, '\0');
	HttpResponse httpResponse;

	removeCharsFromString(value, "\"'");

	if (!value.empty())
	{
		if (errorCode >= 300 && errorCode < 400)
		{
			value =  trim(value);
			std::string tokenWithSlash = value[0] == '/' ? value :  "/" + value;
			tokenWithSlash = trim(tokenWithSlash);
			httpResponse.setVersion("HTTP/1.1");
			httpResponse.setStatusCode(errorCode);
			httpResponse.setHeader("Content-Length", "0");
			httpResponse.setHeader("Connection", "close");
			httpResponse.setHeader("Location", tokenWithSlash);
			httpResponse.setHeader("Server", "LULUGINX");
			response = httpResponse.generateResponse();
		}
		else
		{
			httpResponse.setVersion("HTTP/1.1");
			httpResponse.setStatusCode(errorCode);
			httpResponse.setHeader("Content-Type", "text/html");
			httpResponse.setHeader("Content-Length", intToString(value.size()));
			httpResponse.setHeader("Server", "LULUGINX");
			httpResponse.setBody(value);
			response = httpResponse.generateResponse();
		}
	}
	else
		setErrorCode(errorCode, "[handleRedirect]\t\t Redirect error");
	return true;
}

void Response::handleGET(bool isGet, std::string &uri, bool isHEAD)
{
	if (isGet)
	{
		bool flag = false;
		std::string fullPath = generateFullPath(targetRoute.root, uri, targetRoute.routeName);
		if(handleRedirect(this->targetRoute.redirect != "", targetRoute.redirect))
			return;
		int type = checkType(fullPath);
		if (type == IS_FILE)
			generateResponseFromFile(fullPath, isHEAD);
		else if (type == IS_DIR)
			handleDirectory(fullPath, uri, flag);
	}
}

void Response::handlePOST(bool isPost, std::string &uri, Body &body)
{
	if (isPost)
	{
		HttpResponse httpResponse;

		if (body.getContent().empty())
			return (setErrorCode(400, "[handlePOST]\t\t Empty body"));

		std::string fullPath = generateFullPath(targetRoute.root, uri, targetRoute.routeName);
		if (fullPath.empty())
			return (setErrorCode(404, "[handlePOST]\t\t File not found"));
		if (fullPath[fullPath.size() - 1] != '/')
			fullPath += "/";
		std::ofstream file((fullPath + getCurrentTimestamp()).c_str());
		if (!file.is_open())
			return (setErrorCode(500, "[handlePOST]\t\t Could not open file"));
		file << body.getContent();
		if (!file.good())
			return (setErrorCode(500, "[handlePOST]\t\t Could not write to file"));
		file.close();

		std::string body  = "<center> <h2>File uploaded successfully</h2></center>";
		httpResponse.setVersion("HTTP/1.1");
		httpResponse.setStatusCode(200);
		httpResponse.setHeader("Content-Type", "text/html");
		httpResponse.setHeader("Content-Length", intToString(body.size()));
		httpResponse.setHeader("Connection", "keep-alive");
		if (this->cookies != "")
			httpResponse.setHeader("Set-Cookie", this->cookies);
		httpResponse.setHeader("Server", "LULUGINX");
		httpResponse.setBody(body);
		response = httpResponse.generateResponse();
	}
}

void Response::handleDELETE(bool isDelete, std::string &uri)
{
	if (isDelete)
	{
		HttpResponse httpResponse;
		std::string fullPath = generateFullPath(targetRoute.root, uri, targetRoute.routeName);

		int type = checkType(fullPath);
		if (type == IS_DIR)
			setErrorCode(403, "[handleDELETE]\t\t Cannot delete directory");
		if (remove(fullPath.c_str()) != 0)
			return (setErrorCode(500, "[handleDELETE]\t\t Could not delete file"));
		std::string body = "<center> <h2>File deleted successfully</h2></center>";
		httpResponse.setVersion("HTTP/1.1");
		httpResponse.setStatusCode(200);
		httpResponse.setHeader("Content-Type", "text/html");
		httpResponse.setHeader("Content-Length", intToString(body.size()));
		httpResponse.setHeader("Connection", "keep-alive");
		if (this->cookies != "")
			httpResponse.setHeader("Set-Cookie", this->cookies);
		httpResponse.setHeader("Server", "LULUGINX");
		httpResponse.setBody(body);
		response = httpResponse.generateResponse();
	}
}

void Response::handlePUT(bool isPut, std::string &uri, Body &body)
{
	if (isPut)
	{
		HttpResponse httpResponse;
		std::string fullPath = generateFullPath(targetRoute.root, uri, targetRoute.routeName);

		if (body.getContent().empty())
			return (setErrorCode(400, "[handlePUT]\t\t Empty body"));
		std::ofstream file(fullPath.c_str());
		if (!file.is_open())
			return (setErrorCode(500, "[handlePUT]\t\t Could not open file"));
		file << body.getContent();
		if (!file.good())
			return (setErrorCode(500, "[handlePUT]\t\t Could not write to file"));
		file.close();

		std::string body = "<center> <h2>File updated successfully</h2></center>";
		httpResponse.setVersion("HTTP/1.1");
		httpResponse.setStatusCode(200);
		httpResponse.setHeader("Content-Type", "text/html");
		httpResponse.setHeader("Content-Length", intToString(body.size()));
		httpResponse.setHeader("Connection", "keep-alive");
		if (this->cookies != "")
			httpResponse.setHeader("Set-Cookie", this->cookies);
		httpResponse.setHeader("Server", "LULUGINX");
		httpResponse.setBody(body);
		response = httpResponse.generateResponse();
	}
}

void Response::handleResponse(Request &request)
{
	this->cookies = request.getHeaders().getValue("Cookie");
	std::string method = request.getHeaders().getValue("method");
	std::string uri = request.getHeaders().getValue("uri");
	Body &body = request.getBody();
	if (this->errorCode != 0)
		return (sendError(intToString(this->errorCode)));

	handleGET(method == "GET" || method == "HEAD", uri, method == "HEAD");
	handlePOST(method == "POST", uri, body);
	handleDELETE(method == "DELETE", uri);
	handlePUT(method == "PUT", uri, body);
}

void Response::defaultErrorPage(std::string errorCode)
{
    std::string body = "<html><head><title>Error " + errorCode + " " + getStatusMsg(errorCode) + "</title></head>"
                       "<body style='background-color:lime; color:purple; font-family:Comic Sans MS;'>"
                       "<center><h1 style='font-size:50px; border:5px dotted red;'>Oops! Error " + errorCode + " " + getStatusMsg(errorCode) +"</h1></center>"
                       "<center><p style='font-size:30px; border:3px dashed orange;'>Something went terribly wrong!</p></center>"
                       "<marquee behavior='scroll' direction='left' style='font-size:20px; color:blue;'>This is an ugly error page!</marquee>"
                       "</body></html>";
	HttpResponse httpResponse;
	httpResponse.setVersion("HTTP/1.1");
	httpResponse.setStatusCode(std::atoi(errorCode.c_str()));
	httpResponse.setHeader("Content-Type", "text/html");
	httpResponse.setHeader("Content-Length", intToString(body.size()));
	if (isClosingCode(errorCode))
		httpResponse.setHeader("Connection", "close");
	else
	{
		httpResponse.setHeader("Connection", "keep-alive");
		if (this->cookies != "")
			httpResponse.setHeader("Set-Cookie", this->cookies);
	}
	httpResponse.setHeader("Server", "LULUGINX");
	httpResponse.setBody(body);
	response = httpResponse.generateResponse();
}

void Response::findErrorPage(std::string errorCode, std::map<std::string, std::string> errorPages)
{
	std::map<std::string, std::string>::iterator it = errorPages.find(errorCode);
	if (it != errorPages.end())
	{
		HttpResponse httpResponse;

		std::ifstream file(("." + it->second).c_str());
		std::stringstream buffer;
		buffer << file.rdbuf();
		std::string body = buffer.str();
		httpResponse.setVersion("HTTP/1.1");
		httpResponse.setStatusCode(std::atoi(errorCode.c_str()));
		httpResponse.setHeader("Content-Type", "text/html");
		httpResponse.setHeader("Content-Length", intToString(body.size()));
		if (isClosingCode(errorCode))
			httpResponse.setHeader("Connection", "close");
		else
		{
			if (this->cookies != "")
				httpResponse.setHeader("Set-Cookie", this->cookies);
			httpResponse.setHeader("Connection", "keep-alive");
		}
		httpResponse.setHeader("Server", "LULUGINX");
		httpResponse.setBody(body);
		response = httpResponse.generateResponse();
	}
	else
		defaultErrorPage(errorCode);
}

bool Response::isClosingCode(std::string errorCode)
{
	std::vector<std::string> closeCodes;
	closeCodes.push_back("400"); // Bad Request
	closeCodes.push_back("403"); // Forbidden
	closeCodes.push_back("404"); // Not Found
	closeCodes.push_back("405"); // Method Not Allowed
	closeCodes.push_back("408"); // Request Timeout
	closeCodes.push_back("411"); // Length Required
	closeCodes.push_back("413"); // Payload Too Large
	closeCodes.push_back("414"); // URI Too Long
	closeCodes.push_back("426"); // Upgrade Required
	closeCodes.push_back("431"); // Request Header Fields Too Large
	closeCodes.push_back("499"); // Client Closed Request
	closeCodes.push_back("500"); // Internal Server Error
	closeCodes.push_back("501"); // Not Implemented
	closeCodes.push_back("502"); // Bad Gateway
	closeCodes.push_back("503"); // Service Unavailable
	closeCodes.push_back("504"); // Gateway Timeout
	closeCodes.push_back("505"); // HTTP Version Not Supported

	return (std::find(closeCodes.begin(), closeCodes.end(), errorCode) != closeCodes.end());
}
void Response::sendError(std::string errorCode)
{
	if (isClosingCode(errorCode))
		isConnectionClosed = true;
	
	std::map<std::string, std::string> errorPages = targetServer.errorPages;
	findErrorPage(errorCode, errorPages);
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

bool Response::getIsConnectionClosed() const
{
	return isConnectionClosed;
}

void Response::printResponse()
{
	std::cout << "Response: " << response << std::endl;
}

Response::Response() : errorCode(0), isConnectionClosed(false) { }

Response::Response(const Response &c) 
: 
 statusCodes(c.statusCodes), response(c.response),
  targetServer(c.targetServer), targetRoute(c.targetRoute),
	errorCode(c.errorCode), isConnectionClosed(c.isConnectionClosed), cookies(c.cookies) { }

Response& Response::operator=(const Response &c)
{
	if (this == &c)
		return *this;
	statusCodes = c.statusCodes;
	this->isConnectionClosed = c.isConnectionClosed;
	response = c.response;
	targetServer = c.targetServer;
	targetRoute = c.targetRoute;
	errorCode = c.errorCode;
	return *this;
}

int Response::getErrorCode() const
{
	return (this->errorCode);
}

void Response::setErrorCode(int errorStatus, std::string errorMsg)
{
	this->errorCode = errorStatus;
	throw std::runtime_error(errorMsg);
}

void Response::setIsConnectionClosed(bool isClosed)
{
	isConnectionClosed = isClosed;
}