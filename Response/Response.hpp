/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmunir <nmunir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/03 15:21:58 by nmunir            #+#    #+#             */
/*   Updated: 2024/07/10 16:37:11 by nmunir           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include "../parsing/Parser.hpp"
#include <dirent.h>
#include <cstring>
#include "../request/Request.hpp"

class Response
{
	public:
		Response(int clientFd);
		~Response();
		std::string getResponse();
		void printResponse();
		void setTargetServer(ServerConfig server);
		void setTargetRoute(RouteConfig route);
		ServerConfig getTargetServer();
		RouteConfig getTargetRoute();
		void sendError(std::string errorCode);
		void handleResponse(Request &request);

	private:
		std::map<int, std::string> statusCodes;
		std::string response;
		int clientSocket;
		ServerConfig targetServer;
		RouteConfig targetRoute;
		
		void defaultErrorPage(std::string errorCode);
		void handleGET(bool isGet, RouteConfig &targetRoute, std::string &path);
		void handlePOST(bool isPost, RouteConfig &targetRoute, std::string &path, Body &body);
		void findErrorPage(std::string errorCode, std::map<std::string, std::string> errorPages);
		
		int checkType(std::string &path, RouteConfig &targetRoute);
		bool handleDirectory(std::string &fullPath, std::string &path, RouteConfig &targetRoute);

};

#endif // RESPONSE_HPP