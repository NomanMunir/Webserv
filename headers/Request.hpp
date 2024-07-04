/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmunir <nmunir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/02 12:25:54 by nmunir            #+#    #+#             */
/*   Updated: 2024/07/03 17:20:06 by nmunir           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_HPP
# define REQUEST_HPP

// #include "Body.hpp"
#include "Headers.hpp"
#include "../utils/utils.hpp"
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <sstream>

class Headers;
class Request
{
	public:
		Request(int clientSocket);
		~Request();
	private:
		void handleRequest(int clientSocket);

		std::string request;
		Headers headers;
		// Body body;
};

#endif // REQUEST_HPP