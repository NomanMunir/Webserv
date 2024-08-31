/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abashir <abashir@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/02 12:25:54 by nmunir            #+#    #+#             */
/*   Updated: 2024/08/08 17:52:17 by abashir          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#ifndef REQUEST_HPP
# define REQUEST_HPP

#include "../utils/utils.hpp"
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <sstream>
#include "./Body.hpp"

#define MAX_BODY_SIZE 1000000 // 1MB

class Request
{
	public:
		Request();
		~Request();
		Request(const Request &c);
		Request& operator=(const Request &c);

		bool isComplete() const;
		Headers& getHeaders();
		Body& getBody();
		bool getIsCGI() const;
		std::vector<std::string>& getSystemENV();

		void setComplete(bool complete);
		void reset();
		bool isChunked();
		void handleRequest(ServerConfig &serverConfig, Response &structResponse);
		bool isBodyExist(ServerConfig &serverConfig, Response &structResponse, int fd);

	private:
	
		void findServer(Response &structResponse, Parser &parser);
		bool chooseRoute(std::string path, ServerConfig server, RouteConfig &targetRoute);
		bool checkIsCGI(std::string uri, std::string method, ServerConfig &targetServer);
		void createSystemENV(ServerConfig &serverConfig);

		std::string request;
		std::string rawData;
		Headers headers;
		Body body;
		bool complete;
		bool isCGI;
		std::vector<std::string> systemEnv;
};

#endif // REQUEST_HPP