/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connections.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmunir <nmunir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/20 15:49:37 by nmunir            #+#    #+#             */
/*   Updated: 2024/07/20 17:26:07 by nmunir           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONNECTIONS_HPP
#define CONNECTIONS_HPP

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <sys/event.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <vector>
#include "parsing/Parser.hpp"

#define MAX_EVENTS 100

class Connections
{
	private:
		int serverSocket;
		struct sockaddr_in serverAddr;
		int kqueueFd;
		struct kevent change;
		struct kevent events[MAX_EVENTS];


		void setClient(int fd);
		void removeClient(int fd);
		bool addClient();
		bool peekRequest(int clientSocket);
		bool handleClient(int clientSocket, Parser &configFile);


	public:
		Connections(int fd);
		~Connections();

		void loop(Parser &configFile);
		
};

#endif // CONNECTIONS_HPP