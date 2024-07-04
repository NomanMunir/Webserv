/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmunir <nmunir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/02 12:25:54 by nmunir            #+#    #+#             */
/*   Updated: 2024/07/04 11:33:12 by nmunir           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_HPP
# define REQUEST_HPP

#include "../utils/utils.hpp"
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <sstream>
#include "./Headers.hpp"
#include "./Body.hpp"

class Request
{
	public:
		Request(int clientSocket);
		~Request();
		Headers getHeaders();
		Body getBody();
	private:
		void handleRequest(int clientSocket);

		std::string request;
		Headers headers;
		Body body;
};

#endif // REQUEST_HPP