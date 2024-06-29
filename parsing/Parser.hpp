/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmunir <nmunir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/29 08:52:32 by nmunir            #+#    #+#             */
/*   Updated: 2024/06/29 13:31:49 by nmunir           ###   ########.fr       */
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
#include "utils.hpp"

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
	std::map<std::string, RouteConfig> routeMap;
};

class Parser
{
	public:
		Parser(const std::string configFile);
		~Parser();

	private:
		void validateServerNames(std::string& value);
		void initializeLimits(std::map<std::string, std::vector<int> > &limits);
		void validateHost(std::string &value);
		bool validateNumber(std::string key, std::string value);
		bool validateErrorPages(std::string &key, std::string &value);
		bool validateDirectory(std::string &value);

	
		void validateServerBlock(std::string& key, std::string& value);
		void validateRouteBlock(std::string& key, std::string& value, RouteConfig &routeConfig);
		void checkLocationBlock(RouteConfig &routeConfig);
		void checkServerBlock();
		void validateBlocks();

		void tokanize(std::stringstream &buffer);
		void checkHttpDirective();
		void checkServerDirective();
		void printDirectives();
		void printServers();
		void reset();
		// void resetRoute();
		
		std::string buffer;
		std::string path;
		std::vector<std::string> tokens;
		std::map<std::string, std::string> directives;
		std::vector<ServerConfig> servers;
		ServerConfig serverConfig;
		// RouteConfig routeConfig;
};

#endif // PARSER_HPP