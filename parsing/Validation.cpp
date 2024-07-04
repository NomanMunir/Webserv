/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Validation.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmunir <nmunir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/30 13:35:44 by nmunir            #+#    #+#             */
/*   Updated: 2024/07/04 15:17:42 by nmunir           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Validation.hpp"

void Validation::validateMethods(std::vector<std::string> methods)
{
    std::string validMethods[] = {"GET", "HEAD", "POST", "PUT", "DELETE", "CONNECT", "OPTIONS", "TRACE", "PATCH"};
    for (size_t i = 0; i < methods.size(); i++)
    {
        if (std::find(std::begin(validMethods), std::end(validMethods), methods[i]) == std::end(validMethods))
            throw std::runtime_error("Error: invalid configuration file " + methods[i]);
    }
}

void Validation::validateRouteMap(std::map<std::string, RouteConfig> &routeMap)
{
	for (std::map<std::string, RouteConfig>::iterator it = routeMap.begin(); it != routeMap.end(); it++)
	{
		isDirectory(it->first);
        validateMethods(it->second.methods);

		isDirectory(it->second.root);
		isDirectory(it->second.root + "/" + it->second.redirect);
		isDirectory(it->second.cgiPath);
		isDirectory(it->second.uploadDir);
		isFile(it->second.root + "/" + it->second.defaultFile);
	}
}

bool Validation::validateErrorPages(std::string key, std::string value)
{
	if (!validateNumber("error_pages", key))
		return false;
    if (value.find("/Users/nmunir/Desktop/webserv/") == std::string::npos)
    	value = "/Users/nmunir/Desktop/webserv/" + value;
    isFile(value);        
    return true;
}

void Validation::validateServerNames(std::string &value)
{
    std::stringstream ss(value);
    std::string serverName;
    while (ss >> serverName)
    {
        if (serverName.empty())
            throw std::runtime_error("Error: invalid configuration file " + serverName);
        if (serverName.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.-") != std::string::npos)
            throw std::runtime_error("Error: invalid configuration file " + serverName);
        if (serverName.find("*") != std::string::npos && serverName.find("*") != 0)
            throw std::runtime_error("Error: invalid configuration file " + serverName);
        if (serverName.front() == '-' || serverName.back() == '-' || serverName.front() == '.' || serverName.back() == '.')
            throw std::runtime_error("Error: invalid configuration file " + serverName);
        if (serverName.find("..") != std::string::npos || serverName.find("--") != std::string::npos)
            throw std::runtime_error("Error: invalid configuration file " + serverName);
        if (std::all_of(serverName.begin(), serverName.end(), ::isdigit))
            throw std::runtime_error("Error: invalid configuration file " + serverName);
    }
}

void Validation::validateIP(std::string ip)
{
	int count = std::count(ip.begin(), ip.end(), '.');
    std::vector<std::string> ipParts = split(ip, '.');

	if (count != 3)
		throw std::runtime_error("Error: invalid configuration file " + ip);
	if (ipParts.empty() || ipParts.size() != 4)
		throw std::runtime_error("Error: invalid configuration file " + ip);
	for (size_t i = 0; i < ipParts.size(); i++)
	{
		if (ipParts[i].empty() || !validateNumber("ip", ipParts[i]))
			throw std::runtime_error("Error: invalid configuration file " + ipParts[i]);
	}
}

void Validation::validateListen(std::vector<std::vector<std::string> > &listenVec)
{
    for (size_t i = 0; i < listenVec.size(); i++)
    {
        if (listenVec[i].size() != 2)
            throw std::runtime_error("Error: invalid configuration file " + listenVec[i][0]);
	    validateIP(listenVec[i][0]);
        if (!validateNumber("listen", listenVec[i][1]))
            throw std::runtime_error("Error: invalid configuration file " + listenVec[i][2]);
    }
}

void Validation::validateDirectives(std::map<std::string, std::string> directives)
{
	if (!validateNumber("Content-Length", directives["client_body_size_limit"]))
		throw std::runtime_error("client_body_size_limit directive is invalid");
	if (!validateNumber("keepalive_timeout", directives["keep_alive_timeout"]))
		throw std::runtime_error("keep_alive_timeout directive is invalid");
}

Validation::Validation(Parser parser)
{
	validateDirectives(parser.getDirectives());
	std::vector<ServerConfig> servers = parser.getServers();
	for (size_t i = 0; i < servers.size(); i++)
	{
		validateListen(servers[i].listen);
		validateServerNames(servers[i].serverName);
		for (std::map<std::string, std::string>::iterator it = servers[i].errorPages.begin(); it != servers[i].errorPages.end(); it++)
		{
			if (!validateErrorPages(it->first, it->second))
				throw std::runtime_error("Error: invalid error pages");
		}
		if (!validateNumber("Content-Length", servers[i].clientBodySizeLimit))
			throw std::runtime_error("Error: invalid client body size limit");
		validateRouteMap(servers[i].routeMap);
	}
}

Validation::~Validation(){ }
