/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Headers.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmunir <nmunir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/02 11:20:23 by nmunir            #+#    #+#             */
/*   Updated: 2024/07/02 16:51:13 by nmunir           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Headers.hpp"


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
	std::string uri = headers["uri"];
	if (uri.find("http://") == 0 || uri.find("https://") == 0)
	{
		if ("http://" + host != uri && "https://" + host != uri && uri != host)
			throw std::runtime_error("9 400 (Bad Request) response and close the connection");
		std::cout << "absolute uri: " << uri.substr(host.size() + 1) << std::endl;
	}
	else
	{
		if (uri.find("/") != 0)
			throw std::runtime_error("10 400 (Bad Request) response and close the connection");
		std::cout << "origin-form uri: " << uri << std::endl;
		headers["uri"] = host + uri;
	}
}

void Headers::parseFirstLine()
{
    std::istringstream requestStream(firstLine);
    std::string method;
    std::string uri;
    std::string version;
	
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
	if ((method != "GET" && method != "POST") || version != "HTTP/1.1")
		throw std::runtime_error("8 501 (Not Implemented)");	
    headers["method"] = method;
    headers["version"] = version;
    headers["uri"] = uri;
}

void Headers::parseHeader(std::string &request)
{
		if (request.substr(request.size() - 4) != "\r\n\r\n")
		throw std::runtime_error("0. 400 (Bad Request) response and close the connection");
	request = request.substr(0, request.size() - 2);
	std::string line;
	std::istringstream iss(request);
	bool startLineParsed = true;
	while (std::getline(iss, firstLine, '\n'))
	{
		if (!trim(firstLine).empty())
			break;
	}
	parseFirstLine();
	while (std::getline(iss, line, '\n'))
	{
		if (startLineParsed && (line[0] == ' ' || line[0] == '\t' || line[0] == '\r'))
			throw std::runtime_error("1. 400 (Bad Request) response and close the connection");
		startLineParsed = false;
		if (line.find('\r') != line.size() - 1)
			throw std::runtime_error("2. 400 (Bad Request) response and close the connection");
		size_t pos = line.find(":");
		if (pos == std::string::npos)
			throw std::runtime_error("3. 400 (Bad Request) response and close the connection");
		std::string key = trim(line.substr(0, pos));
		std::string value = trim(line.substr(pos + 1));
		if (key.empty() || value.empty())
			throw std::runtime_error("4. 400 (Bad Request) response and close the connection");
		headers[key] = value;
	}
	parseRequestURI();
}

Headers::Headers(std::string request)
{
	parseHeader(request);
	
	printHeaders();
}

Headers::~Headers() { }