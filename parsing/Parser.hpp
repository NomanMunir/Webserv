/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: absalem < absalem@student.42abudhabi.ae    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/29 08:52:32 by nmunir            #+#    #+#             */
/*   Updated: 2024/08/15 13:20:25 by absalem          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSER_HPP
#define PARSER_HPP

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
#include <algorithm>
#include "../utils/utils.hpp"

#define DEFAULT_ROOT "./"
struct RouteConfig
{
	std::vector<std::string> methods;
	std::string redirect;
	std::string root;
	bool directoryListing;
	std::vector<std::string> defaultFile;
	std::string uploadDir;
	std::vector<std::string> cgiExtensions;
	std::string routeName;
};

struct ServerConfig
{
	std::string root;
	std::vector<std::string> defaultFile;
	std::string cgi_directory;
	std::vector<std::string> cgiExtensions;
	std::vector<std::vector<std::string> > listen;
	std::vector<std::string> serverName;
	std::map<std::string, std::string> errorPages;
	std::string clientBodySizeLimit;
	std::map<std::string, RouteConfig> routeMap;
	std::string env;
};

class Parser
{
	public:
		Parser(const std::string configFile);
		~Parser();
		std::vector<ServerConfig> getServers();
		std::map<std::string, std::string> getDirectives();
		std::vector<std::string> getPorts();
		void setEnv(char **env);

	private:
		void setServerBlock(std::string &key, std::string &value);
		void setRouteBlock(std::string &key, std::string &value);
		void setListen(std::string &value);
		void checkLocationBlock();
		void checkServerBlock();
		
		void checkHttpDirective();
		void checkServerDirective();
		
		void parseBlocks();
		void tokanize(std::stringstream &buffer);
		void setDefault();
		void setDefaultRoute();
		void reset();
		void resetRoute();
		
		
		std::string buffer;
		std::string path;
		std::vector<std::string> tokens;
		std::map<std::string, std::string> directives;
		std::vector<ServerConfig> servers;
		ServerConfig serverConfig;
		RouteConfig routeConfig;
};

#endif // PARSER_HPP