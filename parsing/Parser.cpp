/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmunir <nmunir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/29 08:52:26 by nmunir            #+#    #+#             */
/*   Updated: 2024/07/22 17:45:44 by nmunir           ###   ########.fr       */
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
        if (key == "return")
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
        "methods", "return", "root", "directory_listing",
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
    {
        std::stringstream ss(value);
        std::string token;
        while (std::getline(ss, token, ' '))
        {
            if (token.empty())
                continue;
            serverConfig.serverName.push_back(token);   
        }
    }
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
        for (std::map<std::string, RouteConfig>::iterator it = serverConfig.routeMap.begin(); it != serverConfig.routeMap.end(); it++)
        {
            if (it->first == value)
                throw std::runtime_error("Error: invalid configuration file " + value);
        }
        setDefaultRoute();
        serverConfig.routeMap[value] = routeConfig;
        resetRoute();
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
    serverConfig.routeMap.erase("");
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

void Parser::setDefaultRoute()
{
    if (routeConfig.methods.empty())
        routeConfig.methods.push_back("GET");
    if (routeConfig.redirect.empty())
        routeConfig.redirect = "";
    if (routeConfig.root.empty())
        routeConfig.root = "/Users/nmunir/Desktop/Webserv";
    if (routeConfig.cgiPath.empty())
        routeConfig.cgiPath = "/Users/nmunir/Desktop/Webserv";
    if (routeConfig.uploadDir.empty())
        routeConfig.uploadDir = "/Users/nmunir/Desktop/Webserv";
}

void Parser::setDefault()
{
    for (size_t i = 0; i < servers.size(); i++)
    {
        if (servers[i].serverName.empty())
            servers[i].serverName.push_back("localhost");
        if (servers[i].listen.empty())
        {
            servers[i].listen.push_back(std::vector<std::string>());
            servers[i].listen[0].push_back("0.0.0.0");
            servers[i].listen[0].push_back("80");
        }
        if (servers[i].errorPages.empty())
        {
            servers[i].errorPages["400"] = "/Users/nmunir/Desktop/Webserv/error.html";
            servers[i].errorPages["404"] = "/Users/nmunir/Desktop/Webserv/error.html";
            servers[i].errorPages["500"] = "/Users/nmunir/Desktop/Webserv/error.html";
        }
        if (servers[i].clientBodySizeLimit.empty())
            servers[i].clientBodySizeLimit = "1";
        if (servers[i].routeMap.empty())
        {
            servers[i].routeMap["/"] = routeConfig;
            servers[i].routeMap[""].methods.push_back("GET");
            servers[i].routeMap[""].root = "/Users/nmunir/Desktop/Webserv";
            servers[i].routeMap[""].directoryListing = false;
            servers[i].routeMap[""].defaultFile.push_back("index.html");
            servers[i].routeMap[""].cgiPath = "/Users/nmunir/Desktop/Webserv";
            servers[i].routeMap[""].uploadDir = "/Users/nmunir/Desktop/Webserv";
            servers[i].routeMap[""].redirect = "";
        }
    }
}

void Parser::resetRoute()
{
    routeConfig.methods.clear();
    routeConfig.redirect = "";
    routeConfig.root = "";
    routeConfig.directoryListing = false;
    routeConfig.defaultFile.clear();
    routeConfig.cgiPath = "";
    routeConfig.uploadDir = "";
    serverConfig.routeMap[""] = routeConfig;
}

void Parser::reset()
{
    serverConfig.listen.clear();
    serverConfig.serverName.clear();
    serverConfig.errorPages.clear();
    serverConfig.clientBodySizeLimit = "";
    serverConfig.routeMap.clear();
    resetRoute();
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
    // setDefault();
    reset();
    parseBlocks();
    setDefault();
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

std::vector<std::string> Parser::getPorts()
{
    std::vector<std::string> ports;
    for (size_t i = 0; i < servers.size(); i++)
    {
        for (size_t j = 0; j < servers[i].listen.size(); j++)
            ports.push_back(servers[i].listen[j][1]);
    }
    return ports;
}