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

#define DEFAULT_ROOT "www"
struct RouteConfig
{
	bool 						directoryListing;
	std::string 				redirect;
	std::string 				root;
	std::string 				routeName;
	std::vector<std::string> 	methods;
	std::vector<std::string> 	defaultFile;
	std::vector<std::string> 	cgiExtensions;
};

struct ServerConfig
{
	std::string 							env;
	std::string 							root;
	std::string 							cgi_directory;
	std::string 							clientBodySizeLimit;
	std::vector<std::string> 				defaultFile;
	std::vector<std::string> 				cgiExtensions;
	std::vector<std::string> 				serverName;
	std::map<std::string, std::string> 		errorPages;
	std::map<std::string, RouteConfig> 		routeMap;
	std::vector<std::vector<std::string> > 	listen;
};

class Parser
{
	public:
		Parser(const std::string configFile);
		~Parser();
		Parser(const Parser &src);
		Parser &operator=(const Parser &src);
		std::vector<ServerConfig> getServers();
		std::map<std::string, std::string> getDirectives();
		std::vector<std::string> getPorts();
		void setEnv(char **env);

	private:
		std::string 						buffer;
		std::string							path;
		ServerConfig						serverConfig;
		RouteConfig							routeConfig;
		std::vector<std::string>			tokens;
		std::vector<ServerConfig>			servers;
		std::map<std::string, std::string> directives;

		void reset();
		void setDefault();
		void resetRoute();
		void setDefaultRoute();
		void setServerBlock(std::string &key, std::string &value);
		void setRouteBlock(std::string &key, std::string &value);
		void setListen(std::string &value);

		void checkLocationBlock();
		void checkServerBlock();
		void checkHttpDirective();
		void checkServerDirective();

		void parseBlocks();
		void tokanize(std::stringstream &buffer);
		
};

#endif // PARSER_HPP