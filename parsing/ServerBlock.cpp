/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerBlock.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmunir <nmunir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/02 10:42:44 by nmunir            #+#    #+#             */
/*   Updated: 2024/07/02 11:15:42 by nmunir           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerBlock.hpp"

void ServerBlock::initServerBlock(std::vector<std::string> &tokens)
{

}

ServerBlock::~ServerBlock() { }

ServerBlock::ServerBlock(std::vector<std::string> &tokens)
{
	try
	{
		listen.clear();
		serverName = "default";
		clientBodySizeLimit = "1024";
		errorPages.clear();
		routeMap.clear();
		initServerBlock(tokens);
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}
	
}

