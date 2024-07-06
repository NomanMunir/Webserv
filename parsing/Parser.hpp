/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmunir <nmunir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/29 08:52:32 by nmunir            #+#    #+#             */
/*   Updated: 2024/07/06 10:52:28 by nmunir           ###   ########.fr       */
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


struct RouteConfig
{
	std::vector<std::string> methods;
	std::string redirect;
	std::string root;
	bool directoryListing;
	std::vector<std::string> defaultFile;
	std::string cgiPath;
	std::string uploadDir;
};

struct ServerConfig
{
	std::vector<std::vector<std::string> > listen;
	std::vector<std::string> serverName;
	std::map<std::string, std::string> errorPages;
	std::string clientBodySizeLimit;
	std::map<std::string, RouteConfig> routeMap;
};

class Parser
{
	public:
		Parser(const std::string configFile);
		~Parser();
		std::vector<ServerConfig> getServers();
		std::map<std::string, std::string> getDirectives();

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
		
		std::string buffer;
		std::string path;
		std::vector<std::string> tokens;
		std::map<std::string, std::string> directives;
		std::vector<ServerConfig> servers;
		ServerConfig serverConfig;
		// std::vector<ServerBlock> serverBlock;
		RouteConfig routeConfig;
};

#endif // PARSER_HPP