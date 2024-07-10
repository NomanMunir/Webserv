/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Body.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmunir <nmunir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/03 14:10:31 by nmunir            #+#    #+#             */
/*   Updated: 2024/07/10 17:57:34 by nmunir           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#ifndef BODY_HPP
#define BODY_HPP

#include "../utils/utils.hpp"
#include "Headers.hpp"
#include <iostream>
#include <vector>
#include <map>
#include <string>

class Body
{
	public:
		Body(int clientSocket, std::string length);
		Body(){};
		~Body();
		Body(const Body &b);
		Body &operator=(const Body &b);
		void printBody();
		std::string getBody();

	private:
		std::string body;
		void parseBody(int clientSocket, std::string &length);
};

#endif // BODY_HPP
