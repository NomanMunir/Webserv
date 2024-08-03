/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Body.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abashir <abashir@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/03 14:10:31 by nmunir            #+#    #+#             */
/*   Updated: 2024/07/29 11:53:12 by abashir          ###   ########.fr       */
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
		Body(std::string rawData);
		Body();
		~Body();
		Body(const Body &b);
		Body &operator=(const Body &b);

		void printBody();
		std::string& getContent();
		// void parseBody(std::string length);
		// void parseChunked();

	private:
		std::string body;
};

#endif // BODY_HPP
