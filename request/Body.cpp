/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Body.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abashir <abashir@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/03 14:11:50 by nmunir            #+#    #+#             */
/*   Updated: 2024/07/29 12:30:34 by abashir          ###   ########.fr       */
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

bool Body::readBody(int clientSocket, int contentLength)
{
	if (!ft_recv_body(clientFd, this->body, contentLength))
		return false;
	return true;
}

void Body::printBody()
{
	std::cout << "Body: " << body << std::endl;
}

bool Body::isComplete() const
{
	return complete;
}



// void Body::parseChunked() {
//     char buffer;
//     std::string chunkSize;
//     std::string chunk;
//     int size = 0;
//     while (recv(clientSocket, &buffer, 1, 0) > 0) {
//         if (buffer == '\r') {
//             recv(clientSocket, &buffer, 1, 0);
//             if (buffer == '\n') {
//                 size = std::stoi(chunkSize, 0, 16);
//                 if (size == 0)
//                     break;
//                 while (recv(clientSocket, &buffer, 1, 0) > 0) {
//                     chunk.append(1, buffer);
//                     if (size == chunk.size())
//                         break;
//                 }
//                 body += chunk;
//                 chunkSize.clear();
//                 chunk.clear();
//             }
//         } else {
//             chunkSize.append(1, buffer);
//         }
//     }
// }

// void Body::parseBody(std::string length)
// {
//     char buffer;
//     // Transfer-Encoding takes precedence over Content-Length.
//     // std::cout << "Length: " << length << std::endl;
//     // std::cout << "Parse Body\n";
//     // std::cout << "socket: " << this->clientSocket << std::endl;
//     if (!length.empty())
//     {
//         while (read(this->clientSocket, &buffer, 1) > 0)
//         {
//             std::cout << "Buffer: " << buffer << std::endl;
//             body.append(1, buffer);
//             if (std::atof(length.c_str()) == body.size())
//                 break;
//         }
//     }
//     std::cout << body << std::endl;
// }

// void Body::parseBody(std::string length) {
//     char buffer;
//     // Transfer-Encoding takes precedence over Content-Length.
//     std::cout << "Length: " << length << std::endl;
//     std::cout << "Parse Body\n";
//     if (!length.empty()) {
//         while (recv(clientSocket, &buffer, 1, 0) > 0) {
//             body.append(1, buffer);
//             if (std::atof(length.c_str()) == body.size())
//                 break;
//         }
//     }
// }

std::string& Body::getContent() { return this->body; }