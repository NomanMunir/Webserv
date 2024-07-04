/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Process.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmunir <nmunir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/04 11:29:30 by nmunir            #+#    #+#             */
/*   Updated: 2024/07/04 11:37:07 by nmunir           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Process.hpp"


void Process::handleGetRequest(Request &request)
{
	Headers headers = request.getHeaders();
	std::string uri = headers.getValue("uri");
	std::string host = headers.getValue("Host");
	std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: 12\r\n\r\nHello, World!";
	sendResponse(request.getClientSocket(), response);
}

Process::Process(Request &request)
{
	Headers headers = request.getHeaders();
	if (headers.getValue("method") == "GET")
		handleGetRequest(request);
	
}