/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Validation.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmunir <nmunir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/30 13:36:21 by nmunir            #+#    #+#             */
/*   Updated: 2024/06/30 17:24:36 by nmunir           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef VALIDATION_HPP
#define VALIDATION_HPP

#include "Parser.hpp"

class Parser;
struct RouteConfig;
struct ServerConfig;

class Validation
{
	public:
		Validation(Parser parser);
		~Validation();
	private:
		void validateServerNames(std::string& value);
		void initializeLimits(std::map<std::string, std::vector<int> > &limits);
		void validateHost(std::string &value);
		bool validateNumber(std::string key, std::string value);
		bool validateErrorPages(std::string key, std::string value);		
		void validateDirectives(std::map<std::string, std::string> directives);
		void validateRouteMap(std::map<std::string, RouteConfig> &routeMap);

};

#endif // VALIDATION_HPP