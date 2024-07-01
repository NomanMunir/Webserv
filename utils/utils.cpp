/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmunir <nmunir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/29 08:46:04 by nmunir            #+#    #+#             */
/*   Updated: 2024/07/01 14:04:15 by nmunir           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "utils.hpp"

std::string trim(const std::string &s)
{
    size_t start = s.find_first_not_of(" \t\r\n");
    size_t end = s.find_last_not_of(" \t\r\n");
    return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
}

std::vector<std::string> split(const std::string &s, char delimiter)
{
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter))
    {
        tokens.push_back(trim(token));
    }
    return tokens;
}

void isDirectory(const std::string &path)
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

void isFile(const std::string &path)
{
    struct stat info;
    if (stat(path.c_str(), &info) != 0)
    {
        switch (errno)
        {
        case EACCES:
            throw std::runtime_error("Permission denied to access " + path);
        case ENOENT:
            throw std::runtime_error("File " + path + " does not exist");
        default:
            throw std::runtime_error("Error accessing " + path + ": " + strerror(errno));
        }
    }
    if (!S_ISREG(info.st_mode))
        throw std::runtime_error(path + " is not a regular file");
}

void printServers(std::vector<ServerConfig> servers)
{
    for (size_t i = 0; i < servers.size(); i++)
    {
        std::cout << "Server " << i << std::endl;
        std::cout << "Listen: " << std::endl;
        for (size_t j = 0; j < servers[i].listen.size(); j++)
        {
            for (size_t k = 0; k < servers[i].listen[j].size(); k++)
                std::cout << servers[i].listen[j][k] << " ";
            std::cout << std::endl;
        }
        std::cout << "Server Name: " << servers[i].serverName << std::endl;
        std::cout << "Client Body Size Limit: " << servers[i].clientBodySizeLimit << std::endl;
        std::cout << "Error Pages: " << std::endl;
        for (std::map<std::string, std::string>::iterator it = servers[i].errorPages.begin(); it != servers[i].errorPages.end(); ++it)
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
