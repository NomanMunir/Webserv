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

		void setComplete(bool complete);
		void reset();
		bool isChunked();
		void handleRequest(ServerConfig &serverConfig, Response &structResponse);
		bool isBodyExist(Parser &parser, Response &structResponse, int fd);

	private:
	
		void findServer(Response &structResponse, Parser &parser);
		bool chooseRoute(std::string path, ServerConfig server, RouteConfig &targetRoute);

		std::string request;
		std::string rawData;
		Headers headers;
		Body body;
		bool complete;
};

#endif // REQUEST_HPP