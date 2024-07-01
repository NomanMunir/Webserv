/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Validation.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmunir <nmunir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/30 13:35:44 by nmunir            #+#    #+#             */
/*   Updated: 2024/07/01 15:16:22 by nmunir           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Validation.hpp"

void Validation::initializeLimits(std::map<std::string, std::vector<int> > &limits)
{
    limits["client_body_size"] = std::vector<int>();
    limits["client_body_size"].push_back(0);
    limits["client_body_size"].push_back(1000000); // 0 to 1,000,000 bytes (1 MB)

    limits["keepalive_timeout"] = std::vector<int>();
    limits["keepalive_timeout"].push_back(0);
    limits["keepalive_timeout"].push_back(3600); // 0 to 3,600 seconds (1 hour)

    limits["listen"] = std::vector<int>();
    limits["listen"].push_back(1);
    limits["listen"].push_back(65535); // 1 to 65,535 (valid port range)

    limits["error_pages"] = std::vector<int>();
    limits["error_pages"].push_back(100);
    limits["error_pages"].push_back(599); // 100 to 599 (valid HTTP status codes)
    limits["ip"] = std::vector<int>();
    limits["ip"].push_back(0);   // 0 to 255 (valid IP range)
    limits["ip"].push_back(255); // 0 to 255 (valid IP range)
}

bool Validation::validateNumber(std::string key, std::string value)
{
    std::map<std::string, std::vector<int> > limits;
    initializeLimits(limits);
    size_t limit = value.back() == ';' ? value.size() - 1 : value.size();
    for (size_t j = 0; j < limit; j++)
    {
        if (!isdigit(value[j]))
            return false;
    }
    double num = std::atof(value.substr(0, limit).c_str());
    if (num < limits[key][0] || num > limits[key][1])
        return false;
    return true;
}

void Validation::validateRouteMap(std::map<std::string, RouteConfig> &routeMap)
{
	for (std::map<std::string, RouteConfig>::iterator it = routeMap.begin(); it != routeMap.end(); it++)
	{
		isDirectory(it->first);
        for (size_t i = 0; i < it->second.methods.size(); i++)
        {
            if (it->second.methods[i] != "GET" && it->second.methods[i] != "POST" && it->second.methods[i] != "DELETE" && it->second.methods[i] != "PUT")
                throw std::runtime_error("Error: invalid configuration file " + it->second.methods[i]);
        }
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
        if (listenVec[i].size() != 3)
            throw std::runtime_error("Error: invalid configuration file " + listenVec[i][0]);
	    validateIP(listenVec[i][1]);
        if (!validateNumber("listen", listenVec[i][2]))
            throw std::runtime_error("Error: invalid configuration file " + listenVec[i][2]);
    }
}

void Validation::validateDirectives(std::map<std::string, std::string> directives)
{
	if (!validateNumber("client_body_size", directives["client_body_size_limit"]))
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
		if (!validateNumber("client_body_size", servers[i].clientBodySizeLimit))
			throw std::runtime_error("Error: invalid client body size limit");
		validateRouteMap(servers[i].routeMap);
	}
}

Validation::~Validation(){ }
