/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmunir <nmunir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/29 08:52:26 by nmunir            #+#    #+#             */
/*   Updated: 2024/07/06 11:47:25 by nmunir           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Parser.hpp"

void Parser::setRouteBlock(std::string &key, std::string &value)
{
    if (value.empty())
        throw std::runtime_error("Error: invalid configuration file " + value);
    if (key == "methods")
        routeConfig.methods = split(value, ' ');
    else
    {
        if (key == "redirect")
            routeConfig.redirect = value;
        else if (key == "root")
            routeConfig.root = value;
        else if (key == "directory_listing")
            routeConfig.directoryListing = value == "on" ? true : false;
        else if (key == "default_file")
            routeConfig.defaultFile = split(value, ' ');
        else if (key == "cgi_path" || key == "cgi")
            routeConfig.cgiPath = value;
        else if (key == "upload_dir")
            routeConfig.uploadDir = value;
    }
}

void Parser::checkLocationBlock()
{
    if (tokens.size() < 2)
        throw std::runtime_error("Error: invalid configuration file " + tokens[0]);
    std::string key = tokens[0];
    std::string value = tokens[1];
    tokens.erase(tokens.begin(), tokens.begin() + 2);
    const std::string keyArray[] = {
        "methods", "redirect", "root", "directory_listing",
        "default_file", "cgi_path", "cgi", "upload_dir"
    };
    std::vector<std::string> keys;
    initializeVector(keys, keyArray, sizeof(keyArray) / sizeof(keyArray[0]));

    if (!myFind(keys, key))
        throw std::runtime_error("Error: invalid configuration file " + key);

    while (value.find(";") == std::string::npos)
    {
        if (tokens.empty())
            throw std::runtime_error("Error: invalid configuration file " + key);
        value += " " + tokens[0];
        tokens.erase(tokens.begin());
    }
    if (value.empty() || value.back() != ';')
        throw std::runtime_error("Error: invalid configuration file " + value);
    else
    {
        value = value.substr(0, value.size() - 1);
        setRouteBlock(key, value);
    }
}

void Parser::setListen(std::string &value)
{
    if (value.empty())
        throw std::runtime_error("Error: invalid configuration file " + value);
    std::vector<std::string> listenValues;
    std::vector<std::string> splitedValues = split(value, ':');
    if (splitedValues.size() == 2)
    {
        listenValues.push_back(splitedValues[0]);   
        listenValues.push_back(splitedValues[1]);
    }   
    else if (splitedValues.size() == 1)
    {
        listenValues.push_back("0.0.0.0");
        listenValues.push_back(splitedValues[0]);
    }
    else
        throw std::runtime_error("Error: invalid configuration file " + value);
    serverConfig.listen.push_back(listenValues);
}
void Parser::setServerBlock(std::string &key, std::string &value)
{
    if (key == "server_names")
         serverConfig.serverName = split(value, ' ');
    else if (key == "listen")
        setListen(value);

    else if (key == "client_body_size")
        serverConfig.clientBodySizeLimit = value;
    else if (key == "error_pages")
    {
        std::vector<std::string> errorPages = split(value, ' ');
        for (size_t i = 0; i < errorPages.size() - 1; i++)
            serverConfig.errorPages[errorPages[i]] = errorPages.back();
    }
}

void Parser::checkServerBlock()
{
    if (tokens.size() < 2)
        throw std::runtime_error("Error: invalid configuration file " + tokens[0]);
    std::string key = tokens[0];
    std::string value = tokens[1];
    tokens.erase(tokens.begin(), tokens.begin() + 2);
    const std::string keyArray[] = {"server_names", "listen",
                          "error_pages", "client_body_size", "location"};
    std::vector<std::string> keys;
    initializeVector(keys, keyArray, sizeof(keyArray) / sizeof(keyArray[0]));
    if (!myFind(keys, key))
        throw std::runtime_error("Error: invalid configuration file " + key);
    while (value.find(";") == std::string::npos && key != "location")
    {
        if (tokens.empty())
            throw std::runtime_error("Error: invalid configuration file " + key);
        value += " " + tokens[0];
        tokens.erase(tokens.begin());
    }
    
    if (key == "location")
    {
        if (tokens[0] != "{")
            throw std::runtime_error("Error: invalid configuration file " + tokens[0]);
        // std::cout << key << " " << value << std::endl;
        tokens.erase(tokens.begin());
        while (tokens[0] != "}")
            checkLocationBlock();
        serverConfig.routeMap[value] = routeConfig;
        tokens.erase(tokens.begin());
    }
    else
    {
        if (!value.empty() || value.back() != ';')
        {
            value = value.substr(0, value.size() - 1);
            setServerBlock(key, value);
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
    setDefault();
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
        if (value.back() != ';')
            throw std::runtime_error("Error: invalid configuration file " + value);
    }
    else
        throw std::runtime_error("Error: invalid configuration file " + key);
    directives[key] = value;
}

void Parser::parseBlocks()
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
            // serverBlock.push_back(ServerBlock(tokens));
            checkServerDirective();
            
        }
        else
            checkHttpDirective();
    }
}

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

void Parser::setDefault()
{
    serverConfig.listen.clear();
    serverConfig.serverName.push_back("localhost");
    serverConfig.errorPages["400"] = "/var/www/html/error.html";
    serverConfig.errorPages["404"] = "/var/www/html/error.html";
    serverConfig.errorPages["500"] = "/var/www/html/error.html";
    serverConfig.clientBodySizeLimit = "1";
    routeConfig.methods.push_back("GET");
    routeConfig.methods.push_back("HEAD");
    routeConfig.root = "/var/www/html";
    routeConfig.directoryListing = false;
    routeConfig.defaultFile.push_back("index.html");
    routeConfig.cgiPath = "/var/www/html";
    routeConfig.uploadDir = "/var/www/html";
    routeConfig.redirect = "/";
    serverConfig.routeMap["/"] = routeConfig;
}

Parser::~Parser() {}
Parser::Parser(const std::string configFile)
{
    std::ifstream file(configFile.c_str());
    if (!file.is_open())
        throw std::runtime_error("Error: could not open file " + configFile);
    std::stringstream ss;
    ss << file.rdbuf();
    file.close();
    buffer = ss.str();
    tokanize(ss);
    if (tokens.empty())
        throw std::runtime_error("Error: invalid configuration file :(");
    setDefault();
    parseBlocks();
    if (servers.empty())
        throw std::runtime_error("Error: invalid configuration file :(");
}

std::vector<ServerConfig> Parser::getServers()
{
    return servers;
}

std::map<std::string, std::string> Parser::getDirectives()
{
    return directives;
}