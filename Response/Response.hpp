/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmunir <nmunir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/03 15:21:58 by nmunir            #+#    #+#             */
/*   Updated: 2024/07/03 15:25:04 by nmunir           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include "../Server.hpp"

class Response
{
	public:
		Response(std::string request);
		~Response();
		void printResponse();
	private:
		std::map<int, std::string> statusCodes;
		std::string response;
		void parseResponse(std::string &request);
};

#endif // RESPONSE_HPP