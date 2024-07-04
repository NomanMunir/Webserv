/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationBlock.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmunir <nmunir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/02 10:43:49 by nmunir            #+#    #+#             */
/*   Updated: 2024/07/02 10:52:43 by nmunir           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "LocationBlock.hpp"

LocationBlock::LocationBlock(std::vector<std::string> &tokens)
{
	methods[0] = "GET";
	redirect = "/";
	root = "/Users/nmunir/Desktop/Webserv";
	directoryListing = false;
	defaultFile = "index.html";
	cgiPath = "/Users/nmunir/Desktop/Webserv/cgi-bin";
	uploadDir = "/Users/nmunir/Desktop/Webserv/upload";
	
}