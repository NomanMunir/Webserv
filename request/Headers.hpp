/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Headers.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmunir <nmunir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/02 11:20:28 by nmunir            #+#    #+#             */
/*   Updated: 2024/07/10 12:49:41 by nmunir           ###   ########.fr       */
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
		Headers(Response &structResponse, std::string &request);
		Headers(){};
		~Headers();
		Headers(const Headers &h);
		Headers &operator=(const Headers &h);
		void printHeaders();
		std::string getValue(std::string key);

	private:
		std::map<std::string, std::string> headers;
		std::string firstLine;
		void parseHeader(Response &structResponse, std::string &request);
		void parseFirstLine(Response &structResponse);
		void parseRequestURI(Response &structResponse);
};

#endif // HEADERS_HPP