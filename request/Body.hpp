/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Body.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmunir <nmunir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/03 14:10:31 by nmunir            #+#    #+#             */
/*   Updated: 2024/07/09 17:08:41 by nmunir           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef BODY_HPP
#define BODY_HPP

#include "../utils/utils.hpp"
#include "Headers.hpp"
#include "Response.hpp"
#include <iostream>
#include <vector>
#include <map>
#include <string>

// class Headers;
class Body
{
	public:
		Body(int clientSocket, Headers &headers, Parser &parser);
		Body(){};
		~Body();
		Body(const Body &b);
		Body &operator=(const Body &b);
		void printBody();
		std::string getBody();
	private:
		std::string body;
		void parseBody(int clientSocket, Headers &headers, Parser &parser);
};

#endif // BODY_HPP
