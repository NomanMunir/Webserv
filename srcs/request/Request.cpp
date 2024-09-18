#include "../response/Response.hpp"

Request::Request() : isCGI(false), complete(false) {}

Request::~Request() { }

Request::Request(const Request &c) 
	: body(c.body), isCGI(c.isCGI), \
	complete(c.complete), headers(c.headers), \
	request(c.request), rawData(c.rawData), systemEnv(c.systemEnv) { }

Request& Request::operator=(const Request &c) 
{
    if (this != &c) 
	{
        body = c.body;
		isCGI = c.isCGI;
        complete = c.complete;
        headers = c.headers;
		request = c.request;
        rawData = c.rawData;
		systemEnv = c.systemEnv;
    }
    return *this;
}

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

bool Request::chooseRoute(std::string uri, ServerConfig server, RouteConfig &targetRoute)
{
	std::map<std::string, RouteConfig> routes = server.routeMap;
	std::string method = this->headers.getValue("method");

	uri = trimChar(uri, '/');
	if (method == "POST")
	{
		std::string postRoute = findMatch(uri, routes);
		if (postRoute != "")
		{
			targetRoute = routes[postRoute];
			targetRoute.routeName = postRoute;
			return true;
		}
		return false;
	}

	std::vector<std::string> splitPath = split(uri, '/');
	for (size_t i = 0; i < splitPath.size(); i++)
	{
		std::string route = findMatch(uri, routes);
		if (route != "")
		{
			targetRoute = routes[route];
			targetRoute.routeName = route;
			return true;
		}
		uri = uri.substr(0, uri.find_last_of('/'));
	}
	if (routes.find("/") != routes.end())
	{
		targetRoute = routes["/"];
		targetRoute.routeName = "/";
		return true;
	}
	return false;
}

bool Request::isBodyExist(ServerConfig &serverConfig, Response &structResponse, int fd)
{
	
	std::string length = headers.getValue("Content-Length");
	std::string encoding = headers.getValue("Transfer-Encoding");
	if (length.empty() && encoding.empty())
	{
		char buffer[1];
		if (recv(fd, buffer, 1, MSG_PEEK) <= 0)
			return false;
		else
			structResponse.setErrorCode(411, "[isBodyExist]\t\t Content-Length or Transfer-Encoding is missing");
	}
	if (!length.empty())
	{
		std::string maxBodySize = serverConfig.clientBodySizeLimit == "" ? "1000000" : serverConfig.clientBodySizeLimit;
		if (!validateNumber("Content-Length", length))
			structResponse.setErrorCode(411,"[isBodyExist]\t\t Invalid Content-Length");
		if (std::atof(length.c_str()) > std::atof(maxBodySize.c_str()))
			structResponse.setErrorCode(413,"[isBodyExist]\t\t Request Entity Too Large");
	}
	if (!encoding.empty())
	{
		if (encoding != "chunked")
			structResponse.setErrorCode(411, "[isBodyExist]\t\t Transfer-Encoding Not Implemented");
		this->body.setIsChunked(true);
	}
	return true;
}

void Request::createSystemENV(ServerConfig &serverConfig)
{
	std::string line;
	if (serverConfig.env.empty())
		return;
	std::stringstream ss(serverConfig.env);
	while (getline(ss, line, '\n'))
		this->systemEnv.push_back(line);
}

void  Request::handleRequest(ServerConfig &serverConfig, Response &structResponse)
{
	std::string method = this->headers.getValue("method");
	structResponse.setTargetServer(serverConfig);
	std::string uri = this->headers.getValue("uri");
	if (uri.find(serverConfig.cgi_directory) != std::string::npos)
	{
		if (method != "GET" && method != "POST")
			structResponse.setErrorCode(405, "[handleRequest]\t\t CGI Method Not Allowed");
		this->isCGI = true;
		createSystemENV(serverConfig);
		Logs::appendLog("INFO", "[handleRequest]\t\t CGI request detected.");
	}
	else
	{
		RouteConfig route;
		if (!chooseRoute(uri, serverConfig, route))
		{
			if (method == "POST" || method == "DELETE" || method == "PUT")
				structResponse.setErrorCode(403, "[handleRequest]\t\t Route not found");
			else
				structResponse.setErrorCode(404, "[handleRequest]\t\t Route not found");
		}
		if ((std::find(route.methods.begin(), route.methods.end(), method) == route.methods.end()))
			structResponse.setErrorCode(405, "[handleRequest]\t\t Method Not Allowed");
		structResponse.setTargetRoute(route);
		Logs::appendLog("INFO", "[handleRequest]\t\t Normal request detected.");
		Logs::appendLog("INFO", "[handleRequest]\t\t Route chosen: " + route.routeName);
	}
	Logs::appendLog("INFO", "[handleRequest]\t\t Request handled successfully.");
	this->complete = true;
}

bool Request::isComplete() const {
    return complete;
}

Headers& Request::getHeaders()
{
	return headers;
}

Body& Request::getBody()
{
	return body;
}

bool Request::getIsCGI() const
{
	return isCGI;
}

std::vector<std::string>& Request::getSystemENV()
{
	return this->systemEnv;
}

void Request::setComplete(bool complete)
{
	this->complete = complete;
}

bool Request::isChunked()
{
	if(headers.getValue("Transfer-Encoding") == "chunked")
		return true;
	return false;
}
