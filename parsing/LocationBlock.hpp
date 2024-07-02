/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationBlock.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmunir <nmunir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/02 10:38:35 by nmunir            #+#    #+#             */
/*   Updated: 2024/07/02 10:52:30 by nmunir           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOCATIONBLOCK_HPP
#define LOCATIONBLOCK_HPP

#include "Parser.hpp"

class LocationBlock
{
	public:
		LocationBlock(std::vector<std::string> &tokens);

	private:
		std::vector<std::string> methods;
		std::string redirect;
		std::string root;
		bool directoryListing;
		std::string defaultFile;
		std::string cgiPath;
		std::string uploadDir;
};


#endif // LOCATIONBLOCK_HPP