#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include <vector>
#include <string>
#include <map>
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <cstdlib>
#include <unistd.h>
struct RouteConfig 
{
    std::string path;
    std::vector<std::string> methods;
    std::string redirect;
    std::string root;
    bool directoryListing;
    std::string defaultFile;
    std::string cgiPath;
    std::string uploadDir;
};

struct ServerConfig
{
    std::string host;
    int port;
    std::string serverName;
    std::map<int, std::string> errorPages;
    int clientBodySizeLimit;
    std::map<std::string, RouteConfig> routes;
};

class Http 
{
	public:
		Http(const std::string& configFile);
		~Http();
		void parseDirective(const std::string &line);
		// void setMaxClientBodySize(size_t size);
		// void setKeepaliveTimeout(int timeout);
		// void addServer(const ServerConfig& server);
		// void parseConfigFile(const std::string &filePath);
		void parseServer(std::ifstream &file);
		void parseRoute(std::ifstream &file, ServerConfig &serverConfig, const std::string &path);
		std::string trim(const std::string &s);
		std::vector<std::string> split(const std::string &s, char delimiter);
		const std::vector<ServerConfig>& getServers();
		const std::map<std::string, std::string>& getDirectives();
	private:
		Http();
		std::map<std::string, std::string> directives;
		std::vector<ServerConfig> servers;
};

#endif // SERVERCONFIG_HPP