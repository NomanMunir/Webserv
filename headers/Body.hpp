/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Body.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmunir <nmunir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/03 14:10:31 by nmunir            #+#    #+#             */
/*   Updated: 2024/07/03 17:14:36 by nmunir           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef BODY_HPP
#define BODY_HPP

#include "../utils/utils.hpp"

class Body
{
	public:
		Body(int clientSocket, Headers headers);
		Body(){};
		~Body();
		Body(const Body &b);
		Body &operator=(const Body &b);
		void printBody();
	private:
		std::string body;
		void parseBody(int clientSocket, Headers &headers);
};

#endif // BODY_HPP
