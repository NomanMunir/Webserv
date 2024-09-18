/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Headers.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abashir <abashir@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/02 11:20:23 by nmunir            #+#    #+#             */
/*   Updated: 2024/08/08 16:13:25 by abashir          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Headers.hpp"
#include "../response/Response.hpp"

Headers::Headers(): complete(false) { }

Headers::~Headers() { }

Headers::Headers(const Headers &h)
{
	headers = h.headers;
	this->query = h.query;
	this->rawHeaders = h.rawHeaders;
	complete = h.complete;
}

Headers &Headers::operator=(const Headers &h)
{
	if (this == &h)
		return *this;
	headers = h.headers;
	this->query = h.query;
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

bool Headers::validateMethod(const std::string &method)
{
	std::string validMethods[] = {"GET", "HEAD", "POST", "DELETE", "PUT"};

	if (method.empty())
		return false;
	for (size_t i = 0; i < sizeof(validMethods) / sizeof(validMethods[0]); i++)
	{
		if (method == validMethods[i])
			return true;
	}
	return false;
}

bool Headers::validateUri(const std::string &uri)
{
	if (uri.empty() || uri.find("/") == std::string::npos)
		return false;
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

std::string decodeURI(const std::string &uri)
{
    std::string decoded;
    char ch;
    for (std::string::size_type i = 0; i < uri.length(); ++i)
    {
        if (uri[i] == '%' && i + 2 < uri.length())
        {
            char hex[3] = { uri[i + 1], uri[i + 2], '\0' };
            ch = static_cast<char>(std::strtol(hex, NULL, 16));
            decoded += ch;
            i += 2;
        }
        else
            decoded += uri[i];
    }
    return decoded;
}

void Headers::parseFirstLine(Response &structResponse, std::istringstream &iss)
{
	std::string firstLine;
	while (std::getline(iss, firstLine, '\n'))
	{
		if (!trim(firstLine).empty())
			break;
	}
	std::istringstream requestStream(firstLine);
	if (firstLine.size() > FIRST_LINE_LIMIT)
		structResponse.setErrorCode(501, "[parseFirstLine]\t\t Request-URI Too Long");
    std::vector<std::string> tokens = split(trim(firstLine), ' ');
	if (tokens.size() != 3)
		structResponse.setErrorCode(400, "[parseFirstLine]\t\t Invalid First Line");
	if (!validateMethod(tokens[0]))
		structResponse.setErrorCode(501, "[parseFirstLine]\t\t Method Not Implemented");
	if (!validateUri(tokens[1]))
		structResponse.setErrorCode(400,"[parseFirstLine]\t\t Invalid URI");
	if (tokens[1].size() > URI_LIMIT)
		structResponse.setErrorCode(414, "[parseFirstLine]\t\t URI Too Long");
	if (!validateVersion(tokens[2]))
		structResponse.setErrorCode(505, "[parseFirstLine]\t\t HTTP Version Not Supported");
    headers["method"] = tokens[0];
	std::string splitedURI = split(tokens[1], '?')[0];
    headers["uri"] = decodeURI(splitedURI);
	if (tokens[1].find("?") != std::string::npos)
		headers["query_string"] = split(tokens[1], '?')[1];
	else
		headers["query_string"] = "";
    headers["Version"] = tokens[2];
}

void Headers::isDuplicateHeader(const std::string &key, Response &structResponse)
{
    std::string criticalHeaders[] = {
        "Host", "Content-Length", "Transfer-Encoding",
        "Connection", "Expect", "User-Agent", 
        "Accept-Encoding", "Accept", "Date", 
        "Authorization", "Referer"
    };

    for (long unsigned int i = 0; i < sizeof(criticalHeaders) / sizeof(criticalHeaders[0]); ++i)
    {
        if (key == criticalHeaders[i])
        {
            if (this->headers.find(key) != this->headers.end())
                structResponse.setErrorCode(400, "[parseHeaderBody]\t\t Duplicate Header: " + key);
        }
    }
}

void Headers::parseHeaderBody(std::istringstream &iss, Response &structResponse)
{
	std::string line;
	while (std::getline(iss, line, '\n'))
	{
		if (line.empty() || isspace(line[0]))
			structResponse.setErrorCode(400,"[parseHeaderBody]\t\t Invalid Header " + line);
		if (line.find('\r') != line.size() - 1)
			structResponse.setErrorCode(400, "[parseHeaderBody]\t\t Invalid Header " + line);
		size_t pos = line.find(":");
		if (pos == std::string::npos)
			structResponse.setErrorCode(400, "[parseHeaderBody]\t\t Invalid Header " + line);
		if (isspace(line[pos - 1]))
			structResponse.setErrorCode(400, "[parseHeaderBody]\t\t Invalid Header space before : " + line);
		std::string key = trim(line.substr(0, pos));
		std::string value = trim(line.substr(pos + 1));

		isDuplicateHeader(key, structResponse);
		if (key == "Host")
		{
			std::vector<std::string> tokens = split(value, ':');
			if (tokens.size() == 2)
			{
				if (!validateNumber("listen", tokens[1]))
					structResponse.setErrorCode(400, "[parseHeaderBody]\t\t Invalid Header Port " + line);
				headers["Port"] = tokens[1];
				value = tokens[0];
			}
			else
				headers["Port"] = "80";
		}
		if (key == "Transfer-Encoding" && value != "chunked")
			structResponse.setErrorCode(501, "[parseHeaderBody]\t\t Transfer-Encoding Not Implemented" + line);
		if (key == "Content-Length" && !validateNumber("Content-Length", value))
			structResponse.setErrorCode(400, "[parseHeaderBody]\t\t Invalid Content-Length " + line);
		if (key.empty() || value.empty())
			structResponse.setErrorCode(400, "[parseHeaderBody]\t\t Invalid Header empty key or value " + line);
		headers[key] = value;
	}
}

void Headers::validateAscii(Response &structResponse)
{
	for (size_t i = 0; i < this->rawHeaders.size(); i++)
	{
		if (!isascii(this->rawHeaders[i]))
			return (structResponse.setErrorCode(400, "[validateAscii]\t\t Invalid ASCII"));
	}
}

void Headers::parseHeader(Response &structResponse)
{
	std::string requestHeader = rawHeaders.substr(0, rawHeaders.size() - 2); // -2 is removing the last \r\n
	std::istringstream iss(requestHeader);
	validateAscii(structResponse);
	parseFirstLine(structResponse, iss);
	parseHeaderBody(iss, structResponse);
	std::string host = headers["Host"];
	if (host.empty())
		structResponse.setErrorCode(400, "[parseHeader]\t\t Host is empty");
	
	Logs::appendLog("INFO", "[parseHeader]\t\t Method: " + headers["method"]);
	Logs::appendLog("INFO", "[parseHeader]\t\t URI: " + headers["uri"]);
	Logs::appendLog("INFO", "[parseHeader]\t\t Headers parsed successfully.");

	complete = true;
}

std::string &Headers::getRawHeaders()
{
	return this->rawHeaders;
}

std::map<std::string, std::string> &Headers::getQuery()
{
	return this->query;
}

bool& Headers::isComplete()
{
	return complete;
}

std::string Headers::getValue(std::string key)
{
	if (headers[key].empty())
		return ("");
	return headers[key];
}
