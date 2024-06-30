/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmunir <nmunir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/29 08:46:20 by nmunir            #+#    #+#             */
/*   Updated: 2024/06/30 13:34:10 by nmunir           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_HPP
#define UTILS_HPP

#include "../Server.hpp"
#include "../parsing/Parser.hpp"
struct ServerConfig;

std::string trim(const std::string &s);
std::vector<std::string> split(const std::string &s, char delimiter);
void isFile(const std::string &path);
void isDirectory(const std::string &path);
void printServers(std::vector<ServerConfig> servers);

#endif // UTILS_HPP	