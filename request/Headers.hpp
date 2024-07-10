/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Headers.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmunir <nmunir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/02 11:20:28 by nmunir            #+#    #+#             */
/*   Updated: 2024/07/10 17:10:29 by nmunir           ###   ########.fr       */
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

class Response;
class Headers
{
	public:
		Headers(int clientSocket, Response &structResponse);
		Headers(){};
		~Headers();
		Headers(const Headers &h);
		Headers &operator=(const Headers &h);
		void printHeaders();
		std::string getValue(std::string key);
		std::string getRawHeaders();
		void parseHeader(Response &structResponse);

	private:
		std::map<std::string, std::string> headers;
		std::string firstLine;
		std::string rawHeaders;
		
		void parseFirstLine(Response &structResponse);
		void parseRequestURI(Response &structResponse);
};

#endif // HEADERS_HPP