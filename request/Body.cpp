/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Body.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmunir <nmunir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/03 14:11:50 by nmunir            #+#    #+#             */
/*   Updated: 2024/07/07 13:41:54 by nmunir           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Body.hpp"

Body::Body(int clientSocket, Headers &headers, Parser &parser)
{
	parseBody(clientSocket, headers, parser);
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

void Body::parseBody(int clientSocket, Headers &headers, Parser &parser)
{
    char buffer;

	if (!headers.getValue("Content-Length").empty())
    {
        std::string length = headers.getValue("Content-Length");
        if (length.empty() || !validateNumber("Content-Length", length))
            throw std::runtime_error("7 400 (Bad Request) response and close the connection" + length);
        if (std::atof(length.c_str()) > std::atof(parser.getDirectives()["client_body_size"].c_str()))
            throw std::runtime_error("7 413 (Payload Too Large) response and close the connection");
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


