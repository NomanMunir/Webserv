/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Headers.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmunir <nmunir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/02 11:20:28 by nmunir            #+#    #+#             */
/*   Updated: 2024/07/03 17:20:03 by nmunir           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HEADERS_HPP
#define HEADERS_HPP

#include "../utils/utils.hpp"
#include <sstream>
#include <iostream>
#include <vector>
#include <map>
#include <string>
class Headers
{
	public:
		Headers(int clientSocket);
		Headers(){};
		~Headers();
		Headers(const Headers &h);
		Headers &operator=(const Headers &h);
		void printHeaders();
		std::string getValue(std::string key);

	private:
		std::map<std::string, std::string> headers;
		std::string firstLine;
		void parseHeader(int clientSocket);
		void parseFirstLine();
		void parseRequestURI();
};

#endif // HEADERS_HPP