/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmunir <nmunir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/02 12:31:21 by nmunir            #+#    #+#             */
/*   Updated: 2024/07/07 13:31:51 by nmunir           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"

void Request::handleRequest(int clientSocket, Parser &parser)
{
	headers = Headers(clientSocket);
	body = Body(clientSocket, headers, parser);
}

Request::Request(int clientSocket, Parser &parser)
{
	if (clientSocket == -1)
	{
		std::cerr << "Error accepting client connection." << std::endl;
		return;
	}
	handleRequest(clientSocket, parser);
}

Headers Request::getHeaders()
{
	return headers;
}

Body Request::getBody()
{
	return body;
}
Request::~Request() { }