/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmunir <nmunir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/29 08:52:26 by nmunir            #+#    #+#             */
/*   Updated: 2024/06/29 13:33:03 by nmunir           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Parser.hpp"

Parser::Parser(const std::string configFile)
{
	try
	{
		std::ifstream file(configFile.c_str());
		if (!file.is_open())
			throw std::runtime_error("Error: could not open file " + configFile);
		std::stringstream ss;
		ss << file.rdbuf();
		file.close();
		buffer = ss.str();
		tokanize(ss);
		validateBlocks();
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << std::endl;
	}
}

Parser::~Parser() { }

void Parser::tokanize(std::stringstream &buffer)
{
    std::string line;
    while (std::getline(buffer, line))
    {
        line = trim(line);
        if (line.empty() || line[0] == '#')
            continue;
        std::string token;
        std::stringstream lineStream(line);
        while (lineStream >> token)
            tokens.push_back(token);
    }
}

bool Parser::validateDirectory(std::string &value)
{
    if (value.empty())
		return false;
    // if (access(value.c_str(), F_OK) == -1)
    //     return false;
	return true;
}

void Parser::validateRouteBlock(std::string& key, std::string& value, RouteConfig &routeConfig)
{
    if (value.empty())
		throw std::runtime_error("Error: invalid configuration file " + value);
    if (key == "methods")
    {
        std::vector<std::string> methods = split(value, ' ');
        for (size_t i = 0; i < methods.size(); i++)
        {
            if (methods[i] != "GET" && methods[i] != "POST" && methods[i] != "DELETE" && methods[i] != "PUT")
				throw std::runtime_error("Error: invalid configuration file " + methods[i]);
        }
		routeConfig.methods = methods;
	}
    else
	{
        if (!validateDirectory(value))
			throw std::runtime_error("Error: invalid configuration file " + value);
		if (key == "path")
			routeConfig.path = value;
		else if (key == "redirect")
			routeConfig.redirect = value;
		else if (key == "root")
			routeConfig.root = value;
		else if (key == "directory_listing")
			routeConfig.directoryListing = value == "on" ? true : false;
		else if (key == "default_file")
			routeConfig.defaultFile = value;
		else if (key == "cgi_path" || key == "cgi")
			routeConfig.cgiPath = value;
		else if (key == "upload_dir")
			routeConfig.uploadDir = value;
	}
}

void Parser::checkLocationBlock(RouteConfig &routeConfig)
{
    if (tokens.size() < 2)
		throw std::runtime_error("Error: invalid configuration file " + tokens[0]);
    std::string key = tokens[0];
    std::string value = tokens[1];
    tokens.erase(tokens.begin(), tokens.begin() + 2);
    const std::string keys[] = {"path", "methods", "redirect", "root", \
	 "directory_listing", "default_file", "cgi_path", "cgi", "upload_dir"};

    while (value.find(";") == std::string::npos)
    {
        if (tokens.empty())
			throw std::runtime_error("Error: invalid configuration file " + key);
        value += " " + tokens[0];
        tokens.erase(tokens.begin());
    }
    if (std::find(std::begin(keys), std::end(keys), key) == std::end(keys))
		throw std::runtime_error("Error: invalid configuration file " + key);
    if (value.back() == ';')
    {
        value = value.substr(0, value.size() - 1);
        validateRouteBlock(key, value, routeConfig);
        // std::cout << key << " " << value << std::endl;
    }
    else
		throw std::runtime_error("Error: invalid configuration file " + value);
}

void Parser::validateServerNames(std::string& value)
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

void Parser::initializeLimits(std::map<std::string, std::vector<int> > &limits) 
{
    limits["client_body_size"] = std::vector<int>();
    limits["client_body_size"].push_back(0); 
    limits["client_body_size"].push_back(1000000);  // 0 to 1,000,000 bytes (1 MB)

    limits["keepalive_timeout"] = std::vector<int>();
    limits["keepalive_timeout"].push_back(0);
    limits["keepalive_timeout"].push_back(3600);  // 0 to 3,600 seconds (1 hour)

    limits["port"] = std::vector<int>();
    limits["port"].push_back(1);
    limits["port"].push_back(65535);  // 1 to 65,535 (valid port range)

    limits["error_pages"] = std::vector<int>();
    limits["error_pages"].push_back(100);
    limits["error_pages"].push_back(599);  // 100 to 599 (valid HTTP status codes)
    limits["ip"] = std::vector<int>();
    limits["ip"].push_back(0); // 0 to 255 (valid IP range)
    limits["ip"].push_back(255);  // 0 to 255 (valid IP range)
}

bool Parser::validateNumber(std::string key, std::string value)
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

void Parser::validateHost(std::string &value)
{
    std::vector<std::string> hostParts = split(value, '.');
    int count = std::count(value.begin(), value.end(), '.');
    if (count != 3)
		throw std::runtime_error("Error: invalid configuration file " + value);
    if (hostParts.empty() || hostParts.size() != 4)
		throw std::runtime_error("Error: invalid configuration file " + value);
    for (size_t i = 0; i < hostParts.size(); i++)
    {
        if (hostParts[i].empty() || !validateNumber("ip", hostParts[i]))
			throw std::runtime_error("Error: invalid configuration file " + hostParts[i]);
    }
}

bool Parser::validateErrorPages(std::string &key, std::string &value)
{
    std::vector<std::string> errorPages = split(value, ' ');
    if (errorPages.size() < 2)
        return false;
    for (size_t i = 0; i < errorPages.size() - 1; i++)
    {
        if (!validateNumber(key, errorPages[i]))
            return false;
    }
    if (!validateDirectory(errorPages.back()))
        return false;
    return true;
}

void Parser::validateServerBlock(std::string& key, std::string& value)
{
    if (key == "server_names")
	{
        validateServerNames(value);
		serverConfig.serverName = value;
	}
    else if (key == "listen" || key == "port")
	{
		validateNumber(key, value);
		serverConfig.port = std::atoi(value.c_str());
	}
    else if (key == "client_body_size")
	{
        validateNumber(key, value);
		serverConfig.clientBodySizeLimit = std::atoi(value.c_str());
	}
    else if (key == "host")
	{
        validateHost(value);
		serverConfig.host = value;
	}
    else if (key == "error_pages")
	{
        if (!validateErrorPages(key, value))
			throw std::runtime_error("Error: invalid configuration file " + value);
		std::vector<std::string> errorPages = split(value, ' ');
		for (size_t i = 0; i < errorPages.size() - 1; i++)
			serverConfig.errorPages[std::atoi(errorPages[i].c_str())] = errorPages.back();
	}
}

void Parser::reset()
{
	serverConfig.host = "";
	serverConfig.port = 0;
	serverConfig.serverName = "";
	serverConfig.errorPages.clear();
	serverConfig.clientBodySizeLimit = 0;
	serverConfig.routeMap.clear();
}

// void Parser::resetRoute()
// {
// 	routeConfig.path = "";
// 	routeConfig.methods.clear();
// 	routeConfig.redirect = "";
// 	routeConfig.root = "";
// 	routeConfig.directoryListing = false;
// 	routeConfig.defaultFile = "";
// 	routeConfig.cgiPath = "";
// 	routeConfig.uploadDir = "";
// }

void Parser::checkServerBlock()
{
    if (tokens.size() < 2)
		throw std::runtime_error("Error: invalid configuration file " + tokens[0]);
    std::string key = tokens[0];
    std::string value = tokens[1];
    tokens.erase(tokens.begin(), tokens.begin() + 2);
    std::string keys[] = {"server_names", "listen", "port", "host", \
	 "error_pages", "client_body_size", "location"};
    while (value.find(";") == std::string::npos && key != "location")
    {
        if (tokens.empty())
			throw std::runtime_error("Error: invalid configuration file " + key);
        value += " " + tokens[0];
        tokens.erase(tokens.begin());
    }

    if (std::find(std::begin(keys), std::end(keys), key) == std::end(keys))
		throw std::runtime_error("Error: invalid configuration file " + key);

    if (key == "location")
    {
        if (tokens[0] != "{")
			throw std::runtime_error("Error: invalid configuration file " + tokens[0]);
        // std::cout << key << " " << value << std::endl;
        tokens.erase(tokens.begin());
		RouteConfig routeConfig;
        validateDirectory(value);
        while (tokens[0] != "}")
            checkLocationBlock(routeConfig);
		serverConfig.routeMap[value] = routeConfig;
        tokens.erase(tokens.begin());
    }
    else
    {
        if (value.back() == ';')
        {
            value = value.substr(0, value.size() - 1);
            validateServerBlock(key, value);
            // std::cout << key << " " << value << std::endl;
        }
        else
			throw std::runtime_error("Error: invalid configuration file " + value);
    }
}

void Parser::checkServerDirective()
{
    if (tokens.size() < 2)
		throw std::runtime_error("Error: invalid configuration file " + tokens[0] + tokens[1]);
    while (tokens[0] != "}")
        checkServerBlock();
	servers.push_back(serverConfig);
	reset();
    tokens.erase(tokens.begin());
}

void Parser::checkHttpDirective()
{
    if (tokens.size() < 2)
		throw std::runtime_error("Error: invalid configuration file " + tokens[0] + tokens[1]);

    const std::string key = tokens[0];
    const std::string value = tokens[1];
    tokens.erase(tokens.begin(), tokens.begin() + 2);

    if (key == "client_body_size" || key == "keepalive_timeout")
    {
        if (value.back() == ';')
        {
            // std::cout << key << " " << value << std::endl;
            if (!validateNumber(key, value))
				throw std::runtime_error("Error: invalid configuration file " + value);
        }
        else
			throw std::runtime_error("Error: invalid configuration file " + value);
    }
    else
		throw std::runtime_error("Error: invalid configuration file " + key);
	directives[key] = value;
}

void Parser::validateBlocks()
{
	if (tokens.empty() || tokens[0] != "http" || tokens[1] != "{")
        throw std::runtime_error("Error: invalid configuration file " + tokens[0] + tokens[1]);
    tokens.erase(tokens.begin(), tokens.begin() + 2);
    if (tokens[tokens.size() - 1] != "}")
		throw std::runtime_error("Error: invalid configuration file " + tokens[tokens.size() - 1]);
    tokens.pop_back();
    while (tokens.size() > 0)
    {
        if (tokens[0] == "server")
        {
            if (tokens[1] != "{")
				throw std::runtime_error("Error: invalid configuration file " + tokens[1]);
            tokens.erase(tokens.begin(), tokens.begin() + 2);
            checkServerDirective();
        }
        else
         	checkHttpDirective();
    }
	// printDirectives();
	printServers();
}


void Parser::printDirectives()
{
	for (std::map<std::string, std::string>::iterator it = directives.begin(); it != directives.end(); ++it)
		std::cout << it->first << " => " << it->second << std::endl;
}

void Parser::printServers()
{
	for (size_t i = 0; i < servers.size(); i++)
	{
		std::cout << "Server " << i << std::endl;
		std::cout << "Host: " << servers[i].host << std::endl;
		std::cout << "Port: " << servers[i].port << std::endl;
		std::cout << "Server Name: " << servers[i].serverName << std::endl;
		std::cout << "Client Body Size Limit: " << servers[i].clientBodySizeLimit << std::endl;
		std::cout << "Error Pages: " << std::endl;
		for (std::map<int, std::string>::iterator it = servers[i].errorPages.begin(); it != servers[i].errorPages.end(); ++it)
			std::cout << it->first << " => " << it->second << std::endl;
		std::cout << "Route Map: " << std::endl;
		for (std::map<std::string, RouteConfig>::iterator it = servers[i].routeMap.begin(); it != servers[i].routeMap.end(); ++it)
		{
			std::cout << "Path: " << it->first << std::endl;
			std::cout << "Methods: ";
			for (size_t j = 0; j < it->second.methods.size(); j++)
				std::cout << it->second.methods[j] << " ";
			std::cout << std::endl;
			std::cout << "Redirect: " << it->second.redirect << std::endl;
			std::cout << "Root: " << it->second.root << std::endl;
			std::cout << "Directory Listing: " << it->second.directoryListing << std::endl;
			std::cout << "Default File: " << it->second.defaultFile << std::endl;
			std::cout << "CGI Path: " << it->second.cgiPath << std::endl;
			std::cout << "Upload Dir: " << it->second.uploadDir << std::endl;
			std::cout << std::endl;
		}
	}
}