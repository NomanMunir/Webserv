/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmunir <nmunir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/03 15:21:58 by nmunir            #+#    #+#             */
/*   Updated: 2024/07/08 17:18:02 by nmunir           ###   ########.fr       */
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
		~Response();
		std::string getResponse();
		void sendResponse(int clinetSocket);
		void printResponse();
	private:
		std::map<int, std::string> statusCodes;
		std::string response;
		
		void response404();
		void handleResponse(Request &request, Parser &configFile);
		void handleGET(bool isGet, Request &request, Parser &configFile);
		int checkType(std::string &path, RouteConfig &targetRoute);
		ServerConfig chooseServer(Request &request, Parser &configFile);
		RouteConfig chooseRoute(std::string path, ServerConfig &server);

};

#endif // RESPONSE_HPP