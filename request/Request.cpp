#include "../response/Response.hpp"

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
			return true;
		}
		return false;
	}

	std::vector<std::string> splitPath = split(uri, '/');
	// std::cout << "splitPath size : " << splitPath.size() << std::endl;
	for (size_t i = 0; i < splitPath.size(); i++)
	{
		// std::cout << " i : " << i << " splitPath : " << splitPath[i] << std::endl;
		std::string route = findMatch(uri, routes);
		if (route != "")
		{
			targetRoute = routes[route];
			return true;
		}
		uri = uri.substr(0, uri.find_last_of('/'));
	}
	if (routes.find("/") != routes.end())
	{
		targetRoute = routes["/"];
		return true;
	}
	return false;
}

void Request::findServer(Response &structResponse, Parser &parser)
{
	std::string host = this->headers.getValue("Host");
	std::string port = this->headers.getValue("Port");

	if (host.empty())
	{
		structResponse.setTargetServer(parser.getServers()[0]);
		structResponse.setErrorCode(400, "Request::findServer : Host is empty");
	}
	structResponse.setTargetServer(chooseServer(parser, host, port));
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
		{
			// while(recv(fd, buffer, 1, 0) > 0){}
			structResponse.setErrorCode(411, "Request::isBodyExist : Content-Length or Transfer-Encoding is missing");
		}
	}
	if (!length.empty())
	{
		std::string maxBodySize = serverConfig.clientBodySizeLimit == "" ? "1000000" : serverConfig.clientBodySizeLimit;
		if (!validateNumber("Content-Length", length))
			structResponse.setErrorCode(411,"Request::isBodyExist : Content-Length is not a number");
		if (std::atof(length.c_str()) > std::atof(maxBodySize.c_str()))
			structResponse.setErrorCode(413,"Request::isBodyExist : Content-Length is too big");
	}
	if (!encoding.empty())
	{
		if (encoding != "chunked")
			structResponse.setErrorCode(411, "Request::isBodyExist : Transfer-Encoding is not chunked");
		this->body.setIsChunked(true);
	}
	return true;
}

void  Request::handleRequest(ServerConfig &serverConfig, Response &structResponse)
{
	// this->findServer(structResponse, parser);
	
	structResponse.setTargetServer(serverConfig);
	RouteConfig route;
	std::string uri = this->headers.getValue("uri");
	if (!chooseRoute(uri, serverConfig, route))
	{
		if (headers.getValue("method") == "POST" || headers.getValue("method") == "DELETE")
			structResponse.setErrorCode(403, "Request::handleRequest : POST or DELETE route not found");
		else
			structResponse.setErrorCode(404, "Request::handleRequest : GET Route not found");
	}
	structResponse.setTargetRoute(route);

	this->complete = true;
}

Request::Request() : complete(false) {}

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
Request::~Request() { }

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

Request::Request(const Request &c) : rawData(c.rawData), headers(c.headers), body(c.body), complete(c.complete) {}

Request& Request::operator=(const Request &c) 
{
    if (this != &c) 
	{
        rawData = c.rawData;
        headers = c.headers;
        body = c.body;
        complete = c.complete;
    }
    return *this;
}

void Request::reset()
{
	rawData.clear();
	complete = false;
}
