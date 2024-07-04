/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Headers.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmunir <nmunir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/02 11:20:23 by nmunir            #+#    #+#             */
/*   Updated: 2024/07/04 14:10:31 by nmunir           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Headers.hpp"


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
	return *this;
}

void Headers::printHeaders()
{
	for (std::map<std::string, std::string>::iterator it = headers.begin(); it != headers.end(); it++)
	{
		std::cout << it->first << ": " << it->second << std::endl;
	}
}

void Headers::parseRequestURI()
{
	std::string host = headers["Host"];
	if (host.empty())
		throw std::runtime_error("11 400 (Bad Request) response and close the connection");
	if (host.find("www.") != 0)
		host = "www." + host;
	std::string uri = headers["uri"];
	if (uri.find("http://") == 0) // Absolute URI
	{
		if ("http://" + host != uri)
			throw std::runtime_error("9 400 (Bad Request) response and close the connection");
		std::cout << "absolute uri: " << uri << std::endl;
	}
	else if (headers["method"] == "OPTIONS" && uri == "*") // Asterisk-form URI
	{
		std::cout << "asterisk-form uri: " << uri << std::endl;
		// headers["uri"] = "http://" + host;
	}
	else if (headers["method"] == "CONNECT") // Authority-form URI
	{
		std::vector<std::string> tokens = split(uri, ':');
		if (tokens[0].find("www.") != 0)
			tokens[0] = "www." + tokens[0];
		if (tokens.size() != 2 || !validateNumber("listen", tokens[1]) || tokens[0] != host)
			throw std::runtime_error("12 400 (Bad Request) response and close the connection");
		std::cout << "connect-form uri: " << uri << std::endl;
		// headers["uri"] = "http://" + host + ":" + tokens[1];
	}
	else if (uri[0] == '/') // Origin-form URI
	{
		std::cout << "origin-form uri: " << uri << std::endl;
		// headers["uri"] = "http://" + host + uri;
	}
	else
		throw std::runtime_error("10 400 (Bad Request) response and close the connection");
}

void Headers::parseFirstLine()
{
    std::istringstream requestStream(firstLine);
    std::string method;
    std::string uri;
    std::string version;
    std::string validMethods[] = {"GET", "HEAD", "POST", "PUT", "DELETE", "CONNECT", "OPTIONS", "TRACE", "PATCH"};

	if (firstLine.size() > 8192)
		throw std::runtime_error("5 501 (Not Implemented)");
    std::vector<std::string> tokens = split(trim(firstLine), ' ');
	if (tokens.size() != 3)
		throw std::runtime_error("6 400 (Bad Request) response and close the connection");
	method = tokens[0];
	uri = tokens[1];
	version = tokens[2];
	if (uri.size() > 2048)
		throw std::runtime_error("7 414 (URI Too Long) status code");
	if (std::find(std::begin(validMethods), std::end(validMethods), method) == std::end(validMethods) || version != "HTTP/1.1")
		throw std::runtime_error("8 501 (Not Implemented)");	
    headers["method"] = method;
    headers["version"] = version;
    headers["uri"] = uri;
}

void Headers::parseHeader(int clientSocket)
{
	std::string request;
    char buffer;

	while (read(clientSocket, &buffer, 1) > 0)
    {
		if (!isascii(buffer))
			throw std::runtime_error("Non-ASCII character found in request header.");
        request.append(1, buffer);
        if (request.find("\r\n\r\n") != std::string::npos)
            break;
    }
	if (request.substr(request.size() - 4) != "\r\n\r\n")
		throw std::runtime_error("0. 400 (Bad Request) response and close the connection");
	request = request.substr(0, request.size() - 2);
	std::string line;
	std::istringstream iss(request);
	while (std::getline(iss, firstLine, '\n'))
	{
		if (!trim(firstLine).empty())
			break;
	}
	parseFirstLine();
	while (std::getline(iss, line, '\n'))
	{
		if (line.empty() || isspace(line[0]))
			throw std::runtime_error("1. 400 (Bad Request) response and close the connection");
		if (line.find('\r') != line.size() - 1)
			throw std::runtime_error("2. 400 (Bad Request) response and close the connection");
		size_t pos = line.find(":");
		if (pos == std::string::npos)
			throw std::runtime_error("3. 400 (Bad Request) response and close the connection");
		if (isspace(line[pos - 1]))
			throw std::runtime_error("4. 400 (Bad Request) response and close the connection");
		std::string key = trim(line.substr(0, pos));
		std::string value = trim(line.substr(pos + 1));
		if (key.empty() || value.empty())
			throw std::runtime_error("5. 400 (Bad Request) response and close the connection");
		headers[key] = value;
	}
	parseRequestURI();
}

Headers::Headers(int clientSocket)
{
	parseHeader(clientSocket);
	// printHeaders();
}

Headers::~Headers() { }

std::string Headers::getValue(std::string key)
{
	if (headers[key].empty())
		return ("");
	return headers[key];
}