/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerBlock.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmunir <nmunir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/02 10:30:59 by nmunir            #+#    #+#             */
/*   Updated: 2024/07/02 10:54:09 by nmunir           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP


#include "Parser.hpp"
#include "LocationBlock.hpp"

class ServerBlock
{
	public:
		ServerBlock(std::vector<std::string> &tokens);
		~ServerBlock();
	private:
		std::vector<std::vector<std::string> > listen;
		std::string serverName;
		std::map<std::string, std::string> errorPages;
		std::string clientBodySizeLimit;
		std::map<std::string, LocationBlock> routeMap;

		void initServerBlock(std::vector<std::string> &tokens);
};

#endif // SERVER_HPP