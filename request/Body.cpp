/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Body.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abashir <abashir@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/03 14:11:50 by nmunir            #+#    #+#             */
/*   Updated: 2024/08/08 17:38:26 by abashir          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Body.hpp"

Body::Body(): body(""), isChunked(false) { }

Body::~Body() { }

std::string& Body::getBody() { return this->body; }

void Body::setIsChunked(bool isChunked) { this->isChunked = isChunked; }

Body::Body(const Body &b)
{
	body = b.body;
	isChunked = b.isChunked;
}

Body &Body::operator=(const Body &b)
{
	if (this == &b)
		return *this;
	body = b.body;
	isChunked = b.isChunked;
    return *this;
}

void Body::printBody()
{
	std::cout << "Body: " << body << std::endl;
}

bool& Body::isComplete()
{
	return complete;
}

std::string& Body::getContent() { return this->body; }