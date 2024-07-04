/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmunir <nmunir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/02 12:31:21 by nmunir            #+#    #+#             */
/*   Updated: 2024/07/04 13:49:01 by nmunir           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"

void Request::handleRequest(int clientSocket)
{
	headers = Headers(clientSocket);
	body = Body(clientSocket, headers);
}

Request::Request(int clientSocket)
{
	if (clientSocket == -1)
	{
		std::cerr << "Error accepting client connection." << std::endl;
		return;
	}
	handleRequest(clientSocket);
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