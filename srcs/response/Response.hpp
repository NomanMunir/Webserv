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

#include "HttpResponse.hpp"
#include "../utils/Logs.hpp"
#include "../parsing/Parser.hpp"
#include "../request/Request.hpp"

#define IS_DIR 1
#define IS_FILE 2

class Response
{
	public:
		Response();
		~Response();
		Response(const Response &c);
		Response& operator=(const Response &c);

		std::string getResponse();
		RouteConfig getTargetRoute();
		ServerConfig getTargetServer();
		int 		getErrorCode() const;
		bool 		getIsConnectionClosed() const;
		void 		printResponse();
		int			getClientSocket() const;

		void 		setErrorCode(int errorStatus, std::string errorMsg);
		void 		setTargetServer(ServerConfig server);
		void 		setTargetRoute(RouteConfig route);
		void 		setIsConnectionClosed(bool isClosed);
		void 		sendError(std::string errorCode);
		void 		setClientSocket(int clientSocket);


		int 		checkType(std::string path);
		bool 		checkDefaultFile(std::string &fullPath, bool isCGI);
		void 		handleResponse(Request &request);
		void 		handleDirectory(std::string &fullPath, std::string &uri, bool &isCGI);
		void 		defaultErrorPage(std::string errorCode);
		std::string generateDirectoryListing(const std::string& dirPath, const std::string& uriPath);

	private:
		std::string response;
		int clientSocket;
		ServerConfig targetServer;
		RouteConfig targetRoute;
		int errorCode;
		bool isConnectionClosed;
		std::string cookies;

		void handleGET(bool isGet, std::string &uri, bool isHEAD);
		void handlePUT(bool isPut, std::string &uri, Body &body);
		void handlePOST(bool isPost, std::string &uri, Body &body);
		void handleDELETE(bool isDelete, std::string &uri);
		bool handleRedirect(bool isRedir, std::string redirect);

		void findErrorPage(std::string errorCode, std::map<std::string, std::string> errorPages);
		bool isClosingCode(std::string errorCode);
		void generateResponseFromFile(std::string &path, bool isHEAD);



};

#endif // RESPONSE_HPP