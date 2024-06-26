/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmunir <nmunir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/26 15:05:43 by nmunir            #+#    #+#             */
/*   Updated: 2024/06/26 16:59:35 by nmunir           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerConfig.hpp"

void initializeLimits(std::map<std::string, std::vector<int> > &limits) 
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

bool countBrackets(std::vector<std::string> &tokens)
{
    int count = 0;
    for (size_t i = 0; i < tokens.size(); i++)
    {
        if (tokens[i] == "{")
            count++;
        else if (tokens[i] == "}")
            count--;
    }
    return count == 0;
}

void tokanize(std::stringstream &ss, std::vector<std::string> &tokens)
{
    std::string line;
    while (std::getline(ss, line))
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

bool validateNumber(std::string key, std::string value)
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

bool validateServerNames(std::string value)
{
    std::stringstream ss(value);
    std::string serverName;
    while (ss >> serverName)
    {        
        if (serverName.empty())
            return false;
        if (serverName.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.-") != std::string::npos)
            return false;
        if (serverName.find("*") != std::string::npos && serverName.find("*") != 0)
            return false;
        if (serverName.front() == '-' || serverName.back() == '-' || serverName.front() == '.' || serverName.back() == '.')
            return false;
        if (serverName.find("..") != std::string::npos || serverName.find("--") != std::string::npos)
            return false;
        if (std::all_of(serverName.begin(), serverName.end(), ::isdigit))
            return false;
    }
    return true;
}

bool validateDirectory(std::string value)
{
    if (value.empty())
        return false;
    // if (access(value.c_str(), F_OK) == -1)
    //     return false;
    return true;
}
bool validateErrorPages(std::string key, std::string value)
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
bool validateHost(std::string value)
{
    std::vector<std::string> hostParts = split(value, '.');
    int count = std::count(value.begin(), value.end(), '.');
    if (count != 3)
        return false;
    if (hostParts.empty() || hostParts.size() != 4)
        return false;
    for (size_t i = 0; i < hostParts.size(); i++)
    {
        if (hostParts[i].empty() || !validateNumber("ip", hostParts[i]))
            return false;
    }
    return true;
}

bool validateServerBlock(std::string key, std::string value)
{
    if (key == "server_names")
    {
        if (!validateServerNames(value))
            return false;
    }
    else if (key == "listen" || key == "port")
    {
        if (!validateNumber(key, value))
            return false;
    }
    else if (key == "client_body_size")
    {
        if (!validateNumber(key, value))
            return false;
    }
    else if (key == "host")
    {
        if (!validateHost(value))
            return false;
    }
    else if (key == "error_pages")
    {
        if (!validateErrorPages(key, value))
            return false;
    }
    return true;
}

bool checkHttpDirective(std::vector<std::string> &tokens)
{
    if (tokens.size() < 2)
        return false;

    const std::string key = tokens[0];
    const std::string value = tokens[1];
    tokens.erase(tokens.begin(), tokens.begin() + 2);

    if (key == "client_body_size" || key == "keepalive_timeout")
    {
        if (value.back() == ';')
        {
            std::cout << key << " " << value << std::endl;
            if (!validateNumber(key, value))
                return false;
        }
        else
            return false;
    }
    else
        return false;
    return true;
}

bool validateRouteBlock(std::string key, std::string value)
{
    if (value.empty())
        return false;
    if (key == "methods")
    {
        std::vector<std::string> methods = split(value, ' ');
        for (size_t i = 0; i < methods.size(); i++)
        {
            if (methods[i] != "GET" && methods[i] != "POST" && methods[i] != "DELETE" && methods[i] != "PUT")
                return false;
        }
    }
    else
    {
        if (!validateDirectory(value))
            return false;
    }
    return true;
}
bool checkLocationBlock(std::vector<std::string> &tokens)
{
    if (tokens.size() < 2)
        return false;
    std::string key = tokens[0];
    std::string value = tokens[1];
    tokens.erase(tokens.begin(), tokens.begin() + 2);
    const std::string keys[] = {"path", "methods", "redirect", "root", "directory_listing", "default_file", "cgi_path", "cgi", "upload_dir"};

    while (value.find(";") == std::string::npos)
    {
        if (tokens.empty())
            return false;
        value += " " + tokens[0];
        tokens.erase(tokens.begin());
    }
    if (std::find(std::begin(keys), std::end(keys), key) == std::end(keys))
        return false;

    if (value.back() == ';')
    {
        value = value.substr(0, value.size() - 1);
        if (!validateRouteBlock(key, value))
            return false;
        std::cout << key << " " << value << std::endl;
    }
    else
        return false;
    return true;
}

bool checkServerBlock(std::vector<std::string> &tokens)
{
    if (tokens.size() < 2)
        return false;
    std::string key = tokens[0];
    std::string value = tokens[1];
    tokens.erase(tokens.begin(), tokens.begin() + 2);
    std::string keys[] = {"server_names", "listen", "port", "host", "error_pages", "client_body_size", "location"};
    while (value.find(";") == std::string::npos && key != "location")
    {
        if (tokens.empty())
            return false;
        value += " " + tokens[0];
        tokens.erase(tokens.begin());
    }
    if (std::find(std::begin(keys), std::end(keys), key) == std::end(keys))
        return false;
    if (key == "location")
    {
        if (tokens[0] != "{")
            return false;
        std::cout << key << " " << value << std::endl;
        tokens.erase(tokens.begin());
        if (!validateDirectory(value))
            return false;
        while (tokens[0] != "}")
        {
            if (!checkLocationBlock(tokens))
                return false;
        }
        tokens.erase(tokens.begin());
    }
    else
    {
        if (value.back() == ';')
        {
            value = value.substr(0, value.size() - 1);
            if (!validateServerBlock(key, value))
                return false;
            std::cout << key << " " << value << std::endl;
        }
        else
            return false;
    }
    return true;
}

bool checkServerDirective(std::vector<std::string> &tokens)
{
    if (tokens.size() < 2)
        return false;
    while (tokens[0] != "}")
    {
        if (!checkServerBlock(tokens))
            return false;
    }
    tokens.erase(tokens.begin());
    return true;
}

bool validateBlocks(std::vector<std::string> tokens)
{
    if (tokens.empty() || tokens[0] != "http" || tokens[1] != "{")
        return false;
    tokens.erase(tokens.begin(), tokens.begin() + 2);
    if (tokens[tokens.size() - 1] != "}")
        return false;
    tokens.pop_back();
    while (tokens.size() > 0)
    {
        if (tokens[0] == "server")
        {
            if (tokens[1] != "{")
                return false;
            tokens.erase(tokens.begin(), tokens.begin() + 2);
            if (!checkServerDirective(tokens))
                return false;
        }
        else
        {
            if (!checkHttpDirective(tokens))
                return false;
        }
    }
    return true;
}
bool parser(std::string filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
        return false;
    std::stringstream ss;
    ss << file.rdbuf();
    file.close();
    std::vector<std::string> tokens;
    tokanize(ss, tokens);
    if (!countBrackets(tokens))
        return false;
    if (!validateBlocks(tokens))
        return false;
    return true;
}
int main() {
    
    try
    {
        if (!parser("config.conf"))
            throw std::runtime_error("Error parsing config file");
        // Http http("config.conf");
        // std::vector<ServerConfig> servers = http.getServers();
        // for (size_t i = 0; i < servers.size(); i++)
        // {
        //     std::cout << "Host: " << servers[i].host << std::endl;
        //     std::cout << "Port: " << servers[i].port << std::endl;
        //     std::cout << "Server Name: " << servers[i].serverName << std::endl;
        //     std::cout << "Error Pages: " << std::endl;
        //     for (auto it = servers[i].errorPages.begin(); it != servers[i].errorPages.end(); it++)
        //     {
        //         std::cout << it->first << " " << it->second << std::endl;
        //     }
        //     std::cout << "Client Body Size Limit: " << servers[i].clientBodySizeLimit << std::endl;
        //     std::cout << "Routes: " << std::endl;
        //     for (auto it = servers[i].routes.begin(); it != servers[i].routes.end(); it++)
        //     {
        //         std::cout << "Path: " << it->first << std::endl;
        //         std::cout << "Methods: ";
        //         for (size_t j = 0; j < it->second.methods.size(); j++)
        //         {
        //             std::cout << it->second.methods[j] << " ";
        //         }
        //         std::cout << std::endl;
        //         std::cout << "Redirect: " << it->second.redirect << std::endl;
        //         std::cout << "Root: " << it->second.root << std::endl;
        //         std::cout << "Directory Listing: " << it->second.directoryListing << std::endl;
        //         std::cout << "Default File: " << it->second.defaultFile << std::endl;
        //         std::cout << "CGI Path: " << it->second.cgiPath << std::endl;
        //         std::cout << "Upload Dir: " << it->second.uploadDir << std::endl;
        //     }
        // }   
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    return 0;
}
