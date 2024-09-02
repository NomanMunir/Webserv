/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Validation.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmunir <nmunir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/30 13:36:21 by nmunir            #+#    #+#             */
/*   Updated: 2024/07/22 17:07:38 by nmunir           ###   ########.fr       */
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
		Validation(Parser &parser);
		~Validation();
	private:
		void validateServerNames(std::string& value);
		void validateListen(std::vector<std::vector<std::string> > &value);
		void validateIP(std::string ip);
		void validateMethods(std::vector<std::string> methods);
		bool validateErrorPages(std::string key, std::string value);		
		void validateDirectives(std::map<std::string, std::string> directives);
		void validateRouteMap(std::map<std::string, RouteConfig> &routeMap);
		void validateReturn(std::string &redirect);
		void validateCgiExtensions(std::vector<std::string> cgiExtensions);

};

#endif // VALIDATION_HPP