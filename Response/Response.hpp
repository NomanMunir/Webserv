/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmunir <nmunir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/03 15:21:58 by nmunir            #+#    #+#             */
/*   Updated: 2024/07/09 17:57:46 by nmunir           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include "../request/Request.hpp"
#include "../parsing/Parser.hpp"
#include <dirent.h>
#include <cstring>

class Response
{
	public:
		Response(Request &request, Parser &configFile);
		Response(std::string errorCode, ServerConfig &server);
		~Response();
		std::string getResponse();
		void sendResponse(int clinetSocket);
		void printResponse();
	private:
		std::map<int, std::string> statusCodes;
		std::string response;
		
		void response404();
		void handleResponse(Request &request, Parser &configFile);
		void handleGET(bool isGet, RouteConfig &targetRoute, std::string &path);
		void handlePOST(bool isPost, RouteConfig &targetRoute, std::string &path, Body &body);
		int checkType(std::string &path, RouteConfig &targetRoute);
		bool handleDirectory(std::string &fullPath, std::string &path, RouteConfig &targetRoute);
		ServerConfig chooseServer(Request &request, Parser &configFile);
		RouteConfig chooseRoute(std::string path, ServerConfig &server);

};

#endif // RESPONSE_HPP