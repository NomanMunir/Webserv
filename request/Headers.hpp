/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Headers.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abashir <abashir@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/02 11:20:28 by nmunir            #+#    #+#             */
/*   Updated: 2024/08/08 15:44:22 by abashir          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#ifndef HEADERS_HPP
#define HEADERS_HPP

#include "../utils/utils.hpp"
#include <sstream>
#include <iostream>
#include <vector>
#include <map>
#include <string>

#define URI_LIMIT 2048
#define FIRST_LINE_LIMIT 8192

class Response;
class Headers
{
	public:
		Headers();
		~Headers();
		Headers(const Headers &h);
		Headers &operator=(const Headers &h);

		void printHeaders();
		std::string getValue(std::string key);
		std::string &getRawHeaders();
		std::map<std::string, std::string> &getQuery();

		void parseHeader(Response &structResponse);
		bool& isComplete();

	private:
		std::map<std::string, std::string> headers;
		std::map<std::string, std::string> query;
		std::string rawHeaders;
		bool complete;
		
		void parseFirstLine(Response &structResponse, std::istringstream &iss);
		void parseHeaderBody(std::istringstream &iss, Response &structResponse);
		void isDuplicateHeader(const std::string &key, Response &structResponse);



		void validateAscii(Response &structResponse);
		bool validateMethod(const std::string &method);
		bool validateUri(const std::string &uri);
		bool validateVersion(const std::string &version);
};

#endif // HEADERS_HPP