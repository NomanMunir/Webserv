/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Headers.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmunir <nmunir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/02 11:20:23 by nmunir            #+#    #+#             */
/*   Updated: 2024/07/02 13:56:26 by nmunir           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Headers.hpp"


void Headers::printHeaders()
{
	for (std::map<std::string, std::string>::iterator it = headers.begin(); it != headers.end(); it++)
	{
		std::cout << it->first << ": " << it->second << std::endl;
	}
}

Headers::Headers(std::string request)
{
	std::string line;
	std::istringstream iss(request);
	bool startLineParsed = true;
	while (std::getline(iss, firstLine, '\n'))
	{
		if (!trim(firstLine).empty())
			break;
	}
	
	while (std::getline(iss, line, '\n'))
	{
		if (startLineParsed && (line[0] == ' ' || line[0] == '\t' || line[0] == '\r'))
			throw std::runtime_error("Invalid header format.");
		startLineParsed = false;
		if (line.find('\r') != line.size() - 1)
			throw std::runtime_error("Invalid header format.");
		size_t pos = line.find(":");
		std::string key = trim(line.substr(0, pos));
		std::string value = trim(line.substr(pos + 1));
		headers[key] = value;
	}
	// printHeaders();
}

Headers::~Headers() { }