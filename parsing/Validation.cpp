/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Validation.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmunir <nmunir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/30 13:35:44 by nmunir            #+#    #+#             */
/*   Updated: 2024/07/22 17:47:41 by nmunir           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Validation.hpp"

void Validation::validateMethods(std::vector<std::string> methods)
{
    std::string validMethodsArr[] = {"GET", "HEAD", "POST", "PUT", "DELETE"};
	std::vector<std::string> validMethods;
	initializeVector(validMethods, validMethodsArr, sizeof(validMethodsArr) / sizeof(validMethodsArr[0]));
    for (size_t i = 0; i < methods.size(); i++)
    {
        if (!myFind(validMethods, methods[i]))
            throw std::runtime_error("[validateMethods]\t\t invalid configuration file " + methods[i]);
    }
}

void Validation::validateReturn(std::string &redirect)
{
	std::vector <std::string> tokens = split(redirect, ' ');
	if (tokens.size() == 0 || !validateNumber("error_pages", tokens[0]))
		throw std::runtime_error("[validateReturn]\t\t invalid configuration file " + redirect);
}

void Validation::validateRouteMap(std::map<std::string, RouteConfig> &routeMap)
{
	if (routeMap.begin()->first == "")
		throw std::runtime_error("[validateRouteMap]\t\t invalid configuration file " + routeMap.begin()->first);

	for (std::map<std::string, RouteConfig>::iterator it = routeMap.begin(); it != routeMap.end(); it++)
	{
        validateMethods(it->second.methods);
		validateCgiExtensions(it->second.cgiExtensions);
		if (it->second.redirect != "")
		{
		// std::cout << "redirect: " << it->second.redirect << std::endl;
			validateReturn(it->second.redirect);
		}
		try
		{
			isDirectory(it->second.root);
			isDirectory(it->second.root + "/" + it->first);
			isDirectory(it->second.uploadDir);
		}
		catch (const std::exception &e)
		{
			throw std::runtime_error("[validateRouteMap]\t\t " + std::string(e.what()));
		}
	}
}

bool Validation::validateErrorPages(std::string key, std::string value, std::string root)
{
	if (!validateNumber("error_pages", key))
		return false;
    if (value.find(root) == std::string::npos)
    	value = root + value;
	try
	{
		const std::string path = value;
		isFile(path);
	}
	catch (const std::exception &e)
	{
		throw std::runtime_error("[validateErrorPages]\t\t " + std::string(e.what()));
	}
	return true;
}

void Validation::validateServerNames(std::string &value)
{
	if (value.empty())
		throw std::runtime_error("[validateServerNames]\t\t invalid configuration file " + value);
	if (value.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.-") != std::string::npos)
		throw std::runtime_error("[validateServerNames]\t\t invalid configuration file " + value);
	if (value.find("*") != std::string::npos && value.find("*") != 0)
		throw std::runtime_error("[validateServerNames]\t\t invalid configuration file " + value);
	if (value.front() == '-' || value.back() == '-' || value.front() == '.' || value.back() == '.')
		throw std::runtime_error("[validateServerNames]\t\t invalid configuration file " + value);
	if (value.find("..") != std::string::npos || value.find("--") != std::string::npos)
		throw std::runtime_error("[validateServerNames]\t\t invalid configuration file " + value);
	if (std::all_of(value.begin(), value.end(), ::isdigit))
		throw std::runtime_error("[validateServerNames]\t\t invalid configuration file " + value);
}

void Validation::validateIP(std::string ip)
{
	int count = std::count(ip.begin(), ip.end(), '.');
    std::vector<std::string> ipParts = split(ip, '.');

	if (count != 3)
		throw std::runtime_error("[validateIP]\t\t invalid configuration file " + ip);
	if (ipParts.empty() || ipParts.size() != 4)
		throw std::runtime_error("[validateIP]\t\t invalid configuration file " + ip);
	for (size_t i = 0; i < ipParts.size(); i++)
	{
		if (ipParts[i].empty() || !validateNumber("ip", ipParts[i]))
			throw std::runtime_error("[validateIP]\t\t invalid configuration file " + ipParts[i]);
	}
}

void Validation::validateListen(std::vector<std::vector<std::string> > &listenVec)
{
    for (size_t i = 0; i < listenVec.size(); i++)
    {
        if (listenVec[i].size() != 2)
            throw std::runtime_error("[validateListen]\t\t invalid configuration file " + listenVec[i][0]);
	    validateIP(listenVec[i][0]);
        if (!validateNumber("listen", listenVec[i][1]))
            throw std::runtime_error("[validateListen]\t\t invalid configuration file " + listenVec[i][2]);
    }
}

void Validation::validateDirectives(std::map<std::string, std::string> directives)
{
	if (!validateNumber("Content-Length", directives["client_body_size_limit"]))
		throw std::runtime_error("[validateDirectives]\t\t client_body_size_limit directive is invalid");
	if (!validateNumber("keepalive_timeout", directives["keep_alive_timeout"]))
		throw std::runtime_error("[validateDirectives]\t\t keep_alive_timeout directive is invalid");
}

void Validation::validateCgiExtensions(std::vector<std::string> cgiExtensions)
{
    std::string validCgiExtensions[] = {".php", ".py", ".sh"};
	if (cgiExtensions.size() == 1 && cgiExtensions[0] == "")
		return;
	for (size_t i = 0; i < cgiExtensions.size(); i++)
	{
		if (cgiExtensions[i].empty() || std::find(validCgiExtensions, validCgiExtensions + 3, cgiExtensions[i]) == validCgiExtensions + 3)
			throw std::runtime_error("[validateCgiExtensions]\t\t invalid configuration file " + cgiExtensions[i]);
	}
}

Validation::Validation(Parser &parser)
{
	validateDirectives(parser.getDirectives());
	std::vector<ServerConfig> servers = parser.getServers();
	for (size_t i = 0; i < servers.size(); i++)
	{
		try 
		{
			isDirectory(servers[i].root);
			const std::string cgiDir = servers[i].root + "/" + servers[i].cgi_directory;
			isDirectory(cgiDir);
		}
		catch (const std::exception &e)
		{
			throw std::runtime_error("[Validation]\t\t " + std::string(e.what()));
		}
		
		validateListen(servers[i].listen);
		for (size_t j = 0; j < servers[i].serverName.size(); j++)
			validateServerNames(servers[i].serverName[j]);
		for (std::map<std::string, std::string>::iterator it = servers[i].errorPages.begin(); it != servers[i].errorPages.end(); it++)
		{
			if (!validateErrorPages(it->first, it->second, servers[i].root))
				throw std::runtime_error("[Validation]\t\t invalid error pages");
		}
		if (!validateNumber("Content-Length", servers[i].clientBodySizeLimit))
			throw std::runtime_error("[Validation]\t\t invalid client body size limit");
		validateCgiExtensions(servers[i].cgiExtensions);
		validateRouteMap(servers[i].routeMap);
	}
	// printServers(servers);
}

Validation::~Validation(){ }

void Validation::isDirectory(const std::string &path)
{
    struct stat info;
    if (stat(path.c_str(), &info) != 0)
    {
        switch (errno)
        {
        case EACCES:
            throw std::runtime_error("Permission denied to access " + path);
        case ENOENT:
            throw std::runtime_error("Directory " + path + " does not exist");
        case ENOTDIR:
            throw std::runtime_error("A component of the path " + path + " is not a directory");
        default:
            throw std::runtime_error("Error accessing " + path + ": " + strerror(errno));
        }
    }
    if (!S_ISDIR(info.st_mode))
        throw std::runtime_error(path + " is not a directory");
}
