/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmunir <nmunir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/29 08:46:04 by nmunir            #+#    #+#             */
/*   Updated: 2024/07/13 10:49:35 by nmunir           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "utils.hpp"

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
bool isDirectory(std::string &path)
{
    struct stat info;
    if (stat(path.c_str(), &info) != 0)
    {
        switch (errno)
        {
        case EACCES:
            return (false);
        case ENOENT:
            return (false);
        case ENOTDIR:
            return (false);
        default:
            return (false);
        }
    }
    if (!S_ISDIR(info.st_mode))
        return (false);
    return (true);
}

bool isFile(std::string &path)
{
    struct stat info;
    if (stat(path.c_str(), &info) != 0)
    {
        switch (errno)
        {
        case EACCES:
            return (false);
        case ENOENT:
            return (false);
        default:
            return (false);
        }
    }
    if (!S_ISREG(info.st_mode))
        return (false);
    return (true);
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
    limits["Content-Length"].push_back(1000000); // 0 to 1,000,000 bytes (1 MB)
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
            std::cout << "Server Name: " << servers[i].serverName[j] << std::endl;
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
            std::cout << "CGI Path: " << it->second.cgiPath << std::endl;
            std::cout << "Upload Dir: " << it->second.uploadDir << std::endl;
            std::cout << std::endl;
        }
    }
}

void responseClient(int fd, std::string response)
{
    write(fd, response.c_str(), response.size());
}

std::string getErrorMsg(std::string errorCode)
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

std::string getMimeType(std::string extention)
{
    std::unordered_map<std::string, std::string> mime_types;

    mime_types.insert(std::make_pair("epub", "application/epub+zip"));
    mime_types.insert(std::make_pair("jar", "application/java-archive"));
    mime_types.insert(std::make_pair("war", "application/java-archive"));
    mime_types.insert(std::make_pair("ear", "application/java-archive"));
    mime_types.insert(std::make_pair("js", "application/javascript"));
    mime_types.insert(std::make_pair("json", "application/json"));
    mime_types.insert(std::make_pair("doc", "application/msword"));
    mime_types.insert(std::make_pair("bin", "application/octet-stream"));
    mime_types.insert(std::make_pair("exe", "application/octet-stream"));
    mime_types.insert(std::make_pair("dll", "application/octet-stream"));
    mime_types.insert(std::make_pair("ogx", "application/ogg"));
    mime_types.insert(std::make_pair("pdf", "application/pdf"));
    mime_types.insert(std::make_pair("rtf", "application/rtf"));
    mime_types.insert(std::make_pair("azw", "application/vnd.amazon.ebook"));
    mime_types.insert(std::make_pair("mpkg", "application/vnd.apple.installer+xml"));
    mime_types.insert(std::make_pair("xul", "application/vnd.mozilla.xul+xml"));
    mime_types.insert(std::make_pair("xls", "application/vnd.ms-excel"));
    mime_types.insert(std::make_pair("eot", "application/vnd.ms-fontobject"));
    mime_types.insert(std::make_pair("ppt", "application/vnd.ms-powerpoint"));
    mime_types.insert(std::make_pair("odp", "application/vnd.oasis.opendocument.presentation"));
    mime_types.insert(std::make_pair("ods", "application/vnd.oasis.opendocument.spreadsheet"));
    mime_types.insert(std::make_pair("odt", "application/vnd.oasis.opendocument.text"));
    mime_types.insert(std::make_pair("pptx", "application/vnd.openxmlformats-officedocument.presentationml.presentation"));
    mime_types.insert(std::make_pair("xlsx", "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"));
    mime_types.insert(std::make_pair("docx", "application/vnd.openxmlformats-officedocument.wordprocessingml.document"));
    mime_types.insert(std::make_pair("rar", "application/vnd.rar"));
    mime_types.insert(std::make_pair("7z", "application/x-7z-compressed"));
    mime_types.insert(std::make_pair("bz", "application/x-bzip"));
    mime_types.insert(std::make_pair("bz2", "application/x-bzip2"));
    mime_types.insert(std::make_pair("csh", "application/x-csh"));
    mime_types.insert(std::make_pair("xhtml", "application/xhtml+xml"));
    mime_types.insert(std::make_pair("xht", "application/xhtml+xml"));
    mime_types.insert(std::make_pair("xml", "application/xml"));
    mime_types.insert(std::make_pair("xsl", "application/xml"));
    mime_types.insert(std::make_pair("dtd", "application/xml-dtd"));
    mime_types.insert(std::make_pair("xslt", "application/xslt+xml"));
    mime_types.insert(std::make_pair("yaml", "application/yaml"));
    mime_types.insert(std::make_pair("yml", "application/yaml"));
    mime_types.insert(std::make_pair("sh", "application/x-sh"));
    mime_types.insert(std::make_pair("swf", "application/x-shockwave-flash"));
    mime_types.insert(std::make_pair("tar", "application/x-tar"));
    mime_types.insert(std::make_pair("xhtml", "application/xhtml+xml"));
    mime_types.insert(std::make_pair("zip", "application/zip"));
    mime_types.insert(std::make_pair("avi", "video/x-msvideo"));
    mime_types.insert(std::make_pair("bmp", "image/bmp"));
    mime_types.insert(std::make_pair("gif", "image/gif"));
    mime_types.insert(std::make_pair("ico", "image/vnd.microsoft.icon"));
    mime_types.insert(std::make_pair("jpeg", "image/jpeg"));
    mime_types.insert(std::make_pair("jpg", "image/jpeg"));
    mime_types.insert(std::make_pair("png", "image/png"));
    mime_types.insert(std::make_pair("svg", "image/svg+xml"));
    mime_types.insert(std::make_pair("tif", "image/tiff"));
    mime_types.insert(std::make_pair("tiff", "image/tiff"));
    mime_types.insert(std::make_pair("webp", "image/webp"));
    mime_types.insert(std::make_pair("ico", "image/vnd.microsoft.icon"));
    mime_types.insert(std::make_pair("otf", "font/otf"));
    mime_types.insert(std::make_pair("ttf", "font/ttf"));
    mime_types.insert(std::make_pair("woff", "font/woff"));
    mime_types.insert(std::make_pair("woff2", "font/woff2"));
    mime_types.insert(std::make_pair("css", "text/css"));
    mime_types.insert(std::make_pair("csv", "text/csv"));
    mime_types.insert(std::make_pair("html", "text/html"));
    mime_types.insert(std::make_pair("htm", "text/html"));
    mime_types.insert(std::make_pair("txt", "text/plain"));
    mime_types.insert(std::make_pair("php", "text/php"));
    mime_types.insert(std::make_pair("py", "text/python"));
    mime_types.insert(std::make_pair("js", "text/javascript"));
    mime_types.insert(std::make_pair("xml", "text/xml"));
    mime_types.insert(std::make_pair("md", "text/markdown"));
    
    std::unordered_map<std::string, std::string>::iterator it = mime_types.find(extention);
    if (it != mime_types.end())
        return it->second;
    return "application/octet-stream";
}