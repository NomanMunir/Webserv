/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abashir <abashir@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/29 08:46:04 by nmunir            #+#    #+#             */
/*   Updated: 2024/08/08 17:04:00 by abashir          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "utils.hpp"

int isFileDir(const std::string &path)
{
    struct stat info;
    if (stat(path.c_str(), &info) != 0)
        return IS_ERR;
    if (S_ISREG(info.st_mode))
        return IS_FILE;
    if (S_ISDIR(info.st_mode))
        return IS_DIR;
    return IS_ERR;
}

std::string generateFullPath(std::string rootPath, std::string path, std::string routeName)
{
    bool flag = false;

    if (!path.empty() && path[path.size() - 1] == '/')
        flag = true;

    if (!routeName.empty() && routeName[0] == '/')
        routeName.erase(0, 1);
    if (!path.empty() && path[0] == '/')
        path.erase(0, 1);
    
    std::string newUri = path.erase(0, routeName.size());

    if (!rootPath.empty() && rootPath[rootPath.size() - 1] == '/')
        rootPath.erase(rootPath.size() - 1);

    std::string fullPath = rootPath;
    
    if (!newUri.empty())
        fullPath = rootPath + "/" + newUri;
    if (!fullPath.empty() && fullPath[fullPath.size() - 1] != '/' && flag)
        fullPath += "/";
    return fullPath;
}

std::string getCurrentTimestamp() 
{
    std::time_t now = std::time(NULL);
    std::tm* localTime = std::localtime(&now);
    char buffer[20];
    std::strftime(buffer, sizeof(buffer), "%Y%m%d%H%M%S", localTime);
    return std::string(buffer);
}

std::string toUpperCase(const std::string &str) 
{
    std::string upperStr = str;
    if (upperStr.empty())
        return upperStr;
    for (size_t i = 0; i < upperStr.length(); i++)
    {
        if (upperStr[i] >= 'a' && upperStr[i] <= 'z')
            upperStr[i] = std::toupper(upperStr[i]);
    }
    return upperStr;
}

std::string trim(const std::string &s)
{
    size_t start = s.find_first_not_of(" \t\r\n");
    size_t end = s.find_last_not_of(" \t\r\n");
    return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
}

std::string trimChar(const std::string s, char c)
{
    size_t start = s.find_first_not_of(c);
    size_t end = s.find_last_not_of(c);
    return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
}

std::string join(const std::vector<std::string> &tokens, char delimiter)
{
    std::string joined;
    for (size_t i = 0; i < tokens.size(); ++i)
    {
        if (i > 0)
            joined += delimiter;
        joined += tokens[i];
    }
    return joined;
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

void removeCharsFromString(std::string &str, std::string charsToRemove)
{
    for (size_t i = 0; i < charsToRemove.size(); i++)
    {
        str.erase(std::remove(str.begin(), str.end(), charsToRemove[i]), str.end());
    }
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

void initializeVector(std::vector<std::string> &v, const std::string s[], size_t size)
{
    for (size_t i = 0; i < size; ++i)
    {
        v.push_back(s[i]);
    }
}

void initializeVector(std::vector<std::string> &v, std::string s[], size_t size)
{
    for (size_t i = 0; i < size; ++i)
    {
        v.push_back(s[i]);
    }
}

bool myFind(std::vector<std::string> &v, std::string &s)
{
    for (size_t i = 0; i < v.size(); i++)
    {
        if (v[i] == s)
            return true;
    }
    return false;
}

static void initializeLimits(std::map<std::string, std::vector<int> > &limits)
{
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
    
    limits["Content-Length"] = std::vector<int>();
    limits["Content-Length"].push_back(0);
    limits["Content-Length"].push_back(2000000000); // 0 to 2,000,000,000 bytes (2 GB)

    limits["num"] = std::vector<int>();
    limits["num"].push_back(0);
    limits["num"].push_back(2147483647);
}

bool validateNumber(std::string key, std::string value)
{
    if (value.empty())
        return false;
    std::map<std::string, std::vector<int> > limits;
    initializeLimits(limits);
    size_t limit = value[value.size() - 1] == ';' ? value.size() - 1 : value.size();
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
        for (size_t j = 0; j < servers[i].serverName.size(); j++)
        {
            std::cout << "Server Name: " << servers[i].serverName[j] << std::endl;
        }
        std::cout << "CGI Extensions: " << std::endl;
        for (size_t k = 0; k < servers[i].cgiExtensions.size(); k++)
            std::cout << servers[i].cgiExtensions[k] << " ";
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
            for (size_t j = 0; j < it->second.defaultFile.size(); j++)
                std::cout << "Default File: " << it->second.defaultFile[j] << std::endl;
            std::cout << std::endl;
        }
    }
}


std::string getStatusMsg(std::string errorCode)
{
	std::map<std::string, std::string> errorMsgs;

	// 1xx Informational
	errorMsgs["100"] = "Continue";
	errorMsgs["101"] = "Switching Protocols";
	errorMsgs["102"] = "Processing";
	errorMsgs["103"] = "Early Hints";

	// 2xx Success
	errorMsgs["200"] = "OK";
	errorMsgs["201"] = "Created";
	errorMsgs["202"] = "Accepted";
	errorMsgs["203"] = "Non-Authoritative Information";
	errorMsgs["204"] = "No Content";
	errorMsgs["205"] = "Reset Content";
	errorMsgs["206"] = "Partial Content";
	errorMsgs["207"] = "Multi-Status";
	errorMsgs["208"] = "Already Reported";
	errorMsgs["226"] = "IM Used";

	// 3xx Redirection
	errorMsgs["300"] = "Multiple Choices";
	errorMsgs["301"] = "Moved Permanently";
	errorMsgs["302"] = "Found";
	errorMsgs["303"] = "See Other";
	errorMsgs["304"] = "Not Modified";
	errorMsgs["305"] = "Use Proxy";
	errorMsgs["306"] = "(Unused)";
	errorMsgs["307"] = "Temporary Redirect";
	errorMsgs["308"] = "Permanent Redirect";

	// 4xx Client Error
	errorMsgs["400"] = "Bad Request";
	errorMsgs["401"] = "Unauthorized";
	errorMsgs["402"] = "Payment Required";
	errorMsgs["403"] = "Forbidden";
	errorMsgs["404"] = "Not Found";
	errorMsgs["405"] = "Method Not Allowed";
	errorMsgs["406"] = "Not Acceptable";
	errorMsgs["407"] = "Proxy Authentication Required";
	errorMsgs["408"] = "Request Timeout";
	errorMsgs["409"] = "Conflict";
	errorMsgs["410"] = "Gone";
	errorMsgs["411"] = "Length Required";
	errorMsgs["412"] = "Precondition Failed";
	errorMsgs["413"] = "Payload Too Large";
	errorMsgs["414"] = "URI Too Long";
	errorMsgs["415"] = "Unsupported Media Type";
	errorMsgs["416"] = "Range Not Satisfiable";
	errorMsgs["417"] = "Expectation Failed";
	errorMsgs["418"] = "I'm a teapot";
	errorMsgs["421"] = "Misdirected Request";
	errorMsgs["422"] = "Unprocessable Entity";
	errorMsgs["423"] = "Locked";
	errorMsgs["424"] = "Failed Dependency";
	errorMsgs["425"] = "Too Early";
	errorMsgs["426"] = "Upgrade Required";
	errorMsgs["428"] = "Precondition Required";
	errorMsgs["429"] = "Too Many Requests";
	errorMsgs["431"] = "Request Header Fields Too Large";
	errorMsgs["451"] = "Unavailable For Legal Reasons";

	// 5xx Server Error
	errorMsgs["500"] = "Internal Server Error";
	errorMsgs["501"] = "Not Implemented";
	errorMsgs["502"] = "Bad Gateway";
	errorMsgs["503"] = "Service Unavailable";
	errorMsgs["504"] = "Gateway Timeout";
	errorMsgs["505"] = "HTTP Version Not Supported";
	errorMsgs["506"] = "Variant Also Negotiates";
	errorMsgs["507"] = "Insufficient Storage";
	errorMsgs["508"] = "Loop Detected";
	errorMsgs["510"] = "Not Extended";
	errorMsgs["511"] = "Network Authentication Required";

    if (errorMsgs.empty())
        return "";
    return (errorMsgs[errorCode]);
}

std::string intToString(int number) 
{
    std::ostringstream oss;
    oss << number;
    return oss.str();
}