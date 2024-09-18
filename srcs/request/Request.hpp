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

#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include "./Body.hpp"
#include "../utils/utils.hpp"
#include "../utils/Logs.hpp"

#define MAX_BODY_SIZE 1000000 // 1MB

class Request
{
	public:
		Request();
		~Request();
		Request(const Request &c);
		Request& operator=(const Request &c);

		bool isChunked();
		bool isComplete() const;
		bool getIsCGI() const;
		Headers& getHeaders();
		Body& getBody();
		std::vector<std::string>& getSystemENV();

		void setComplete(bool complete);
		void handleRequest(ServerConfig &serverConfig, Response &structResponse);
		bool isBodyExist(ServerConfig &serverConfig, Response &structResponse);

	private:
		bool chooseRoute(std::string path, ServerConfig server, RouteConfig &targetRoute);
		void createSystemENV(ServerConfig &serverConfig);

		Body body;
		bool isCGI;
		bool complete;
		Headers headers;
		std::string request;
		std::string rawData;
		std::vector<std::string> systemEnv;
};

#endif // REQUEST_HPP