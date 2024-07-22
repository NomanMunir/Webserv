/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Headers.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmunir <nmunir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/02 11:20:23 by nmunir            #+#    #+#             */
/*   Updated: 2024/07/22 11:02:26 by nmunir           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Headers.hpp"
#include "../response/Response.hpp"

Headers::Headers(const Headers &h)
{
	headers = h.headers;
	firstLine = h.firstLine;
}

Headers &Headers::operator=(const Headers &h)
{
	if (this == &h)
		return *this;
	headers = h.headers;
	firstLine = h.firstLine;
	this->rawHeaders = h.rawHeaders;
	return *this;
}

void Headers::printHeaders()
{
	for (std::map<std::string, std::string>::iterator it = headers.begin(); it != headers.end(); it++)
	{
		std::cout << it->first << ": " << it->second << std::endl;
	}
}

void Headers::parseRequestURI(Response &structResponse)
{
	std::string host = headers["Host"];
	if (host.empty())
		structResponse.sendError("400");
	if (host.find("www.") != 0)
		host = "www." + host;
	std::string uri = headers["uri"];
	if (uri.find("http://") == 0) // Absolute URI
	{
		if ("http://" + host != uri)
			structResponse.sendError("400");
	}
	else if (headers["method"] == "OPTIONS" && uri == "*") // Asterisk-form URI
	{
		// headers["uri"] = "http://" + host;
	}
	else if (headers["method"] == "CONNECT") // Authority-form URI
	{
		std::vector<std::string> tokens = split(uri, ':');
		if (tokens[0].find("www.") != 0)
			tokens[0] = "www." + tokens[0];
		if (tokens.size() != 2 || !validateNumber("listen", tokens[1]) || tokens[0] != host)
			structResponse.sendError("400");
		// headers["uri"] = "http://" + host + ":" + tokens[1];
	}
	else if (uri[0] == '/') // Origin-form URI
	{
		// headers["uri"] = "http://" + host + uri;
	}
	else
		structResponse.sendError("400");
}

void Headers::parseFirstLine(Response &structResponse)
{
    std::istringstream requestStream(firstLine);
    std::string method;
    std::string uri;
    std::string version;
    std::string validMethods[] = {"GET", "HEAD", "POST", "PUT", "DELETE", "CONNECT", "OPTIONS", "TRACE", "PATCH"};

	if (firstLine.size() > 8192)
		structResponse.sendError("501");
    std::vector<std::string> tokens = split(trim(firstLine), ' ');
	if (tokens.size() != 3)
		structResponse.sendError("400");
	method = tokens[0];
	uri = tokens[1];
	version = tokens[2];
	if (uri.size() > 2048)
		structResponse.sendError("414");
	if (std::find(std::begin(validMethods), std::end(validMethods), method) == std::end(validMethods) || version != "HTTP/1.1")
		structResponse.sendError("501");
    headers["method"] = method;
    headers["version"] = version;
	
    headers["uri"] = split(uri, '?')[0];
}

void Headers::parseHeader(Response &structResponse)
{
	std::string request = rawHeaders.substr(0, rawHeaders.size() - 2); // -2 is removing the last \r\n
	std::string line;
	std::istringstream iss(request);
	while (std::getline(iss, firstLine, '\n'))
	{
		if (!trim(firstLine).empty())
			break;
	}
	parseFirstLine(structResponse);
	while (std::getline(iss, line, '\n'))
	{
		if (line.empty() || isspace(line[0]))
			structResponse.sendError("400");
		if (line.find('\r') != line.size() - 1)
			structResponse.sendError("400");
		size_t pos = line.find(":");
		if (pos == std::string::npos)
			structResponse.sendError("400");
		if (isspace(line[pos - 1]))
			structResponse.sendError("400");
		std::string key = trim(line.substr(0, pos));
		std::string value = trim(line.substr(pos + 1));
		if (key == "Host")
		{
			std::vector<std::string> tokens = split(value, ':');
			if (tokens.size() == 2)
			{
				if (!validateNumber("listen", tokens[1]))
					structResponse.sendError("400");
				headers["Port"] = tokens[1];
				value = tokens[0];
			}
			else
				headers["Port"] = "80";
		}
		if (key.empty() || value.empty())
			structResponse.sendError("400");
		headers[key] = value;
	}
	parseRequestURI(structResponse);
}

Headers::Headers(int clientSocket, Response &structResponse)
{
    char buffer;

	while (read(clientSocket, &buffer, 1) > 0)
    {
		if (!isascii(buffer))
			structResponse.sendError("400");
        this->rawHeaders.append(1, buffer);
        if (this->rawHeaders.find("\r\n\r\n") != std::string::npos)
            break;
    }
	// std::cout << "rawHeaders : " << this->rawHeaders << std::endl;
	// parseHeader(structResponse);
}

std::string Headers::getRawHeaders()
{
	return this->rawHeaders;
}
Headers::~Headers() { }

std::string Headers::getValue(std::string key)
{
	if (headers[key].empty())
		return ("");
	return headers[key];
}
