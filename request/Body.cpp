/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Body.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmunir <nmunir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/03 14:11:50 by nmunir            #+#    #+#             */
/*   Updated: 2024/07/04 10:21:54 by nmunir           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Body.hpp"

Body::Body(int clientSocket, Headers &headers)
{
	parseBody(clientSocket, headers);
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

void Body::parseBody(int clientSocket, Headers &headers)
{
    char buffer;

	if (!headers.getValue("Content-Length").empty())
    {
        std::string length = headers.getValue("Content-Length");
        if (length.empty() || !validateNumber("Content-Length", length))
            throw std::runtime_error("7 400 (Bad Request) response and close the connection" + length);
            
        while (read(clientSocket, &buffer, 1) > 0)
        {
            body.append(1, buffer);
            if (std::atof(length.c_str()) == body.size())
                break;
        }
        std::cout << "Body: " << body << std::endl;
    }
    else if (!headers.getValue("Transfer-Encoding").empty())
    {
    
    }
}


