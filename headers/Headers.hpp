/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Headers.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmunir <nmunir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/02 11:20:28 by nmunir            #+#    #+#             */
/*   Updated: 2024/07/02 16:20:07 by nmunir           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HEADERS_HPP
#define HEADERS_HPP

#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <sstream>
#include "../Server.hpp"
class Headers
{
	public:
		Headers(std::string request);
		~Headers();
		void printHeaders();

	private:
		std::map<std::string, std::string> headers;
		std::string firstLine;
		void parseHeader(std::string &request);
		void parseFirstLine();
		void parseRequestURI();
};

#endif // HEADERS_HPP