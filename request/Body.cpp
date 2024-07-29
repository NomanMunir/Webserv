/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Body.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmunir <nmunir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/03 14:11:50 by nmunir            #+#    #+#             */
/*   Updated: 2024/07/15 09:19:08 by nmunir           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Body.hpp"

Body::Body(int fd): body(""), clientSocket(fd) {}

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

void Body::parseChunked() {
    char buffer;
    std::string chunkSize;
    std::string chunk;
    int size = 0;
    while (recv(clientSocket, &buffer, 1, 0) > 0) {
        if (buffer == '\r') {
            recv(clientSocket, &buffer, 1, 0);
            if (buffer == '\n') {
                size = std::stoi(chunkSize, 0, 16);
                if (size == 0)
                    break;
                while (recv(clientSocket, &buffer, 1, 0) > 0) {
                    chunk.append(1, buffer);
                    if (size == chunk.size())
                        break;
                }
                body += chunk;
                chunkSize.clear();
                chunk.clear();
            }
        } else {
            chunkSize.append(1, buffer);
        }
    }
}

void Body::parseBody(std::string length) {
    char buffer;
    // Transfer-Encoding takes precedence over Content-Length.
    std::cout << "Length: " << length << std::endl;
    if (!length.empty()) {
        while (recv(clientSocket, &buffer, 1, 0) > 0) {
            body.append(1, buffer);
            if (std::atof(length.c_str()) == body.size())
                break;
        }
    }
}

std::string Body::getBody() { return body; }