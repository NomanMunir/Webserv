/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmunir <nmunir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/02 12:25:54 by nmunir            #+#    #+#             */
/*   Updated: 2024/07/10 17:56:49 by nmunir           ###   ########.fr       */
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
		void handleRequest(Parser &parser, Response &structResponse);

	private:
		void findServer(Response &structResponse, Parser &parser);
		bool isBodyExist(Parser &parser, Response &structResponse);

		std::string request;
		std::string rawData;
		Headers headers;
		Body body;
		bool complete;
};

#endif // REQUEST_HPP