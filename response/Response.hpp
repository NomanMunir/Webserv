/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: absalem < absalem@student.42abudhabi.ae    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/03 15:21:58 by nmunir            #+#    #+#             */
/*   Updated: 2024/08/15 13:24:32 by absalem          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <dirent.h>
#include <cstring>

#include "../parsing/Parser.hpp"
#include "../request/Request.hpp"
#include "HttpResponse.hpp"

#define DIR_ERR 1
#define FILE_ERR 2


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
		void handleResponse(Request &request, char **env);

		int getErrorCode() const;
		void setErrorCode(int errorStatus, std::string errorMsg);
		bool getIsConnectionClosed() const;

	private:
		std::map<int, std::string> statusCodes;
		std::string response;
		int clientSocket;
		ServerConfig targetServer;
		RouteConfig targetRoute;
		int errorCode;
		bool isConnectionClosed;
		
		void handleGET(bool isGet, std::string &uri);
		void handlePOST(bool isPost, std::string &uri, Body &body);
		void handleDELETE(bool isDelete, std::string &uri);

		void defaultErrorPage(std::string errorCode);
		void findErrorPage(std::string errorCode, std::map<std::string, std::string> errorPages);
		bool isClosingCode(std::string errorCode);


		int checkType(std::string &path, RouteConfig &targetRoute);
		std::string listDirectory(const std::string& dirPath, const std::string& uriPath);
		bool handleDirectory(std::string &fullPath, std::string &path, RouteConfig &targetRoute);
		void generateResponseFromFile(std::string &path);
		void handleRedirect(std::string redirect);



};

#endif // RESPONSE_HPP