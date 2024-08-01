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

	// std::cout << "requestHeader : " << requestHeader << std::endl;
	std::string::size_type hostPos = requestHeader.find("Host: ");
	if (hostPos != std::string::npos)
	{
		std::string::size_type endHostPos = requestHeader.find("\r\n", hostPos);	
		// std::cout << "hi2 \n";
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
		if (!validateNumber("Content-Length", length))
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

void Request::handleRequest(Parser &parser, Response &structResponse) 
{
    try {
        // Step 1: Parse Headers
        this->headers = Headers(rawData, structResponse);
        this->headers.parseHeader(structResponse);

        // Step 2: Determine Target Server
        this->findServer(structResponse, parser);
        ServerConfig server = structResponse.getTargetServer();

        // Step 3: Determine Target Route
        RouteConfig route;
        if (!chooseRoute(headers.getValue("uri"), server, route)) {
            structResponse.sendError("404");
            return;
        }
        structResponse.setTargetRoute(route);

        // Step 4: Parse Body (if present)
        // if (isBodyExistRequest(parser, structResponse)) {
        //     this->body = Body(clientSocket);
        //     if (!headers.getValue("Content-Length").empty())
        //         body.parseBody(headers.getValue("Content-Length"));
        //     else if (headers.getValue("Transfer-Encoding") == "chunked")
        //         body.parseChunked();
        // }

        // headers.printHeaders();
        // body.printBody();

    } catch (const std::exception &e) {
        std::cerr << "Error handling request: " << e.what() << std::endl;
        structResponse.sendError("500"); // Internal Server Error
    }
}

bool Request::appendData(const std::string &data, Response &response, Parser &configFile) 
{
    rawData += data;
    if (!complete)
	{
		std::cout << "rawData : " << rawData << std::endl;
        for (int i = 0; i < rawData.size(); i++)
        {
            if (!isascii(rawData[i]))
                response.sendError("400");
        }
        handleRequest(configFile, response);
        if (headers.isComplete()) 
        {
            complete = true;  // Or set some condition for complete request
        	//     parseBody();
        }
    }
    return complete;
}

Request::Request() : complete(false) {}

bool Request::isComplete() const {
    return complete;
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

void Request::setComplete(bool complete)
{
	this->complete = complete;
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
