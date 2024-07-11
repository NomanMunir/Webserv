/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Body.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmunir <nmunir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/03 14:11:50 by nmunir            #+#    #+#             */
/*   Updated: 2024/07/11 17:28:22 by nmunir           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Body.hpp"

Body::Body(int clientSocket, std::string length)
{
	parseBody(clientSocket, length);
}

Body::~Body() { }

Body::Body(const Body &b)
{
	body = b.body;
}

Body &Body::operator=(const Body &b)
{
	if (this == &b)
		return *this;
	body = b.body;
	return *this;
}

void Body::printBody()
{
	std::cout << "Body: " << body << std::endl;
}

void Body::parseBody(int clientSocket, std::string &length)
{
    char buffer;
    // Transfer-Encoding takes precedence over Content-Length.
    if (!length.empty())
    {
        while (read(clientSocket, &buffer, 1) > 0)
        {
            body.append(1, buffer);
            if (std::atof(length.c_str()) == body.size())
                break;
        }
    }
}

std::string Body::getBody() { return body; }