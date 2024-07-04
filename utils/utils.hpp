/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmunir <nmunir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/29 08:46:20 by nmunir            #+#    #+#             */
/*   Updated: 2024/07/04 11:13:50 by nmunir           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_HPP
#define UTILS_HPP

#include "../parsing/Parser.hpp"
#include <string>
#include <vector>
#include <netinet/in.h>
#include <map>
#include <iostream>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cerrno>

struct ServerConfig;

std::string trim(const std::string &s);
std::vector<std::string> split(const std::string &s, char delimiter);
void isFile(const std::string &path);
void isDirectory(const std::string &path);
void printServers(std::vector<ServerConfig> servers);
bool validateNumber(std::string key, std::string value);

#endif // UTILS_HPP	