/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmunir <nmunir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/02 12:25:54 by nmunir            #+#    #+#             */
/*   Updated: 2024/07/10 13:55:13 by nmunir           ###   ########.fr       */
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
		Request(int clientSocket, Parser &parser, Response &structResponse);
		~Request();
		Headers getHeaders();
		Body getBody();
	private:
		void handleRequest(int clientSocket, Parser &parser, Response &structResponse);

		
		std::string request;
		Headers headers;
		Body body;
};

#endif // REQUEST_HPP