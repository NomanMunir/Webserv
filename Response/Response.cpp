/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmunir <nmunir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/03 15:22:38 by nmunir            #+#    #+#             */
/*   Updated: 2024/07/03 16:14:59 by nmunir           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"

Response::Response(std::string request)
{
}

Response::~Response() { }

void Response::printResponse()
{
	std::cout << "Response: " << response << std::endl;
}

