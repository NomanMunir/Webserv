/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmunir <nmunir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/03 15:21:58 by nmunir            #+#    #+#             */
/*   Updated: 2024/07/22 16:31:55 by nmunir           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <dirent.h>
#include <cstring>

#include "../parsing/Parser.hpp"
#include "../request/Request.hpp"

class Response
{
	public:
		Response();
		~Response();
		Response(const Response &c);
		Response& operator=(const Response &c);


		std::string getResponse();
		void printResponse();
		void setTargetServer(ServerConfig server);
		void setTargetRoute(RouteConfig route);
		ServerConfig getTargetServer();
		RouteConfig getTargetRoute();
		void sendError(std::string errorCode);
		void handleResponse(Request &request);
		bool getIsConClosed();
		void reset();

	private:
		std::map<int, std::string> statusCodes;
		std::string response;
		int clientSocket;
		ServerConfig targetServer;
		RouteConfig targetRoute;
		bool isConClosed;
		
		void defaultErrorPage(std::string errorCode);
		void handleGET(bool isGet, RouteConfig &targetRoute, std::string &path);
		void handlePOST(bool isPost, RouteConfig &targetRoute, std::string &path, Body &body);
		void findErrorPage(std::string errorCode, std::map<std::string, std::string> errorPages);

		int checkType(std::string &path, RouteConfig &targetRoute);
		std::string listDirectory(const std::string& dirPath, const std::string& uriPath);
		bool handleDirectory(std::string &fullPath, std::string &path, RouteConfig &targetRoute);
		void generateResponseFromFile(std::string &path);
		void handleRedirect(std::string redirect);



};

#endif // RESPONSE_HPP