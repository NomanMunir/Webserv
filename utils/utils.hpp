/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmunir <nmunir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/29 08:46:20 by nmunir            #+#    #+#             */
/*   Updated: 2024/07/20 14:17:50 by nmunir           ###   ########.fr       */
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
#include <unordered_map>

struct ServerConfig;

std::string trim(const std::string &s);
std::string join(const std::vector<std::string> &tokens, char delimiter);
std::string trimChar(const std::string s, char c);
void removeCharsFromString(std::string &str, std::string charsToRemove);
std::vector<std::string> split(const std::string &s, char delimiter);
void isFile(const std::string &path);
bool isFile(std::string &path);
void isDirectory(const std::string &path);
bool isDirectory(std::string &path);
void printServers(std::vector<ServerConfig> servers);
bool validateNumber(std::string key, std::string value);
bool myFind(std::vector<std::string> &v, std::string &s);
void initializeVector(std::vector<std::string> &v, const std::string s[], size_t size);
void initializeVector(std::vector<std::string> &v, std::string s[], size_t size);
std::string getErrorMsg(std::string errorCode);
bool responseClient(int fd, std::string response);
std::string getMimeType(std::string extention);
bool ft_recv_header(int fd, std::string &buffer);
bool ft_recv_body(int fd, std::string &buffer, size_t contentLength);


#endif // UTILS_HPP	