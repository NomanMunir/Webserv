/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Body.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abashir <abashir@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/03 14:10:31 by nmunir            #+#    #+#             */
/*   Updated: 2024/08/08 17:38:36 by abashir          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#ifndef BODY_HPP
#define BODY_HPP

#include <map>
#include <vector>
#include <string>
#include <iostream>
#include "Headers.hpp"
#include "../utils/utils.hpp"

class Body
{
	public:
		Body();
		~Body();
		Body(const Body &b);
		Body &operator=(const Body &b);

		std::string& getBody();
		std::string& getContent();

		void printBody();
		bool& isComplete();
		void setIsChunked(bool isChunked);

	private:
		bool 		complete;
		bool 		isChunked;
		std::string body;

};

#endif // BODY_HPP
