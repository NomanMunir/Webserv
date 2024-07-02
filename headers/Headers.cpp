/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Headers.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmunir <nmunir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/02 11:20:23 by nmunir            #+#    #+#             */
/*   Updated: 2024/07/02 14:59:39 by nmunir           ###   ########.fr       */
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

void Headers::parseFirstLine()
{
    std::istringstream requestStream(firstLine);
    std::string method;
    std::string uri;
    std::string version;

    std::vector<std::string> tokens = split(firstLine, ' ');
	if (tokens.size() != 3)
		throw std::runtime_error("Invalid header format.5");
	method = tokens[0];
	uri = tokens[1];
	version = tokens[2];
	if ((method != "GET" && method != "POST") || version != "HTTP/1.1")
		throw std::runtime_error("Invalid header format.6");		
    headers["method"] = method;
    headers["version"] = version;
    headers["uri"] = uri;
}

void Headers::parseHeader(std::string &request)
{
		if (request.substr(request.size() - 4) != "\r\n\r\n")
		throw std::runtime_error("Invalid header format.0");
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
			throw std::runtime_error("Invalid header format.1");
		startLineParsed = false;
		if (line.find('\r') != line.size() - 1)
			throw std::runtime_error("Invalid header format.2");
		size_t pos = line.find(":");
		if (pos == std::string::npos)
			throw std::runtime_error("Invalid header format.3");
		std::string key = trim(line.substr(0, pos));
		std::string value = trim(line.substr(pos + 1));
		if (key.empty() || value.empty())
			throw std::runtime_error("Invalid header format.4");
		headers[key] = value;
	}
}

Headers::Headers(std::string request)
{
	parseHeader(request);
	
	printHeaders();
}

Headers::~Headers() { }