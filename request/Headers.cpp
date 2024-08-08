/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Headers.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abashir <abashir@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/02 11:20:23 by nmunir            #+#    #+#             */
/*   Updated: 2024/07/29 12:06:55 by abashir          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Headers.hpp"
#include "../response/Response.hpp"

Headers::Headers(const Headers &h)
{
	headers = h.headers;
	firstLine = h.firstLine;
	this->rawHeaders = h.rawHeaders;
	complete = h.complete;
}

Headers &Headers::operator=(const Headers &h)
{
	if (this == &h)
		return *this;
	headers = h.headers;
	firstLine = h.firstLine;
	this->rawHeaders = h.rawHeaders;
	complete = h.complete;
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

bool Headers::validateMethod(const std::string &method)
{
	std::string validMethods[] = {"GET", "HEAD", "POST", "DELETE"};
	if (method.empty())
		return false;
	for (size_t i = 0; i < validMethods->size(); i++)
	{
		if (method == validMethods[i])
			return true;
	}
	return false;
}

bool Headers::validateQuery(const std::string &uri)
{
	std::string query = uri.substr(uri.find("?") + 1);
	if (query.empty())
		return false;
	std::vector<std::string> tokens = split(query, '&');
	for (size_t i = 0; i < tokens.size(); i++)
	{
		std::vector<std::string> pair = split(tokens[i], '=');
		if (pair.size() != 2 || pair[0].empty() || pair[1].empty())
			return false;
		this->query[pair[0]] = pair[1];
	}
	return true;
}

bool Headers::validateUri(const std::string &uri)
{
	if (uri.empty() || uri.find("/") == std::string::npos)
		return false;
	if (uri.find("?") != std::string::npos)
	{
		if (!validateQuery(uri))
			return false;
	}
	return true;
}

bool Headers::validateVersion(const std::string &version)
{
	if (version.empty() || version.find("HTTP/") != 0)
		return (false);
	std::vector<std::string> tokens = split(version, '/');
	if (tokens.size() != 2 || tokens[1].empty())
		return (false);
	if (!isdigit(tokens[1][0]) || tokens[1][1] != '.' || !isdigit(tokens[1][2]))
		return (false);
	float ver = std::atof(tokens[1].c_str());
	if (ver >= 1 && ver <= 1.9)
		return (true);
	return (false);
}

void Headers::parseFirstLine(Response &structResponse)
{
    std::istringstream requestStream(firstLine);

	if (firstLine.size() > FIRST_LINE_LIMIT)
		structResponse.sendError("501");
    std::vector<std::string> tokens = split(trim(firstLine), ' ');
	if (tokens.size() != 3)
	{

		std::cout << "Zing Zing: " << tokens.size() << std::endl;
		structResponse.sendError("400");
	}
	if (!validateMethod(tokens[0]))
		structResponse.sendError("405");
	if (!validateUri(tokens[1]))
		structResponse.sendError("400");
	if (tokens[1].size() > URI_LIMIT)
		structResponse.sendError("414");
	if (!validateVersion(tokens[2]))
		structResponse.sendError("505");
    headers["method"] = tokens[0];
    headers["uri"] = split(tokens[1], '?')[0];
    headers["version"] = tokens[2];
}

void Headers::parseHeader(Response &structResponse)
{
	std::string request = rawHeaders.substr(0, rawHeaders.size() - 2); // -2 is removing the last \r\n
	std::string line;
	std::istringstream iss(request);

	for (size_t i = 0; i < request.size(); i++)
	{
		if (!isascii(this->rawHeaders[i]))
			structResponse.sendError("400");
	}

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
		if (key == "Transfer-Encoding" && value != "chunked")
			structResponse.sendError("501");
		if (key == "Content-Length" && !validateNumber("Content-Length", value))
			structResponse.sendError("400");
		if (key.empty() || value.empty())
			structResponse.sendError("400");
		headers[key] = value;
	}
	parseRequestURI(structResponse);
	complete = true;
}

Headers::Headers(std::string &rawData, Response &structResponse) : complete(false)
{

	if (rawData.find("\r\n\r\n") == std::string::npos)
		structResponse.sendError("400");
	else
		this->rawHeaders = rawData.substr(0, rawData.find("\r\n\r\n") + 4);
}

std::string &Headers::getRawHeaders()
{
	return this->rawHeaders;
}

std::map<std::string, std::string> &Headers::getQuery()
{
	return this->query;
}

bool Headers::isComplete() const
{
	return complete;
}

Headers::~Headers() { }

std::string Headers::getValue(std::string key)
{
	if (headers[key].empty())
		return ("");
	return headers[key];
}
