/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abashir <abashir@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/08 17:07:42 by nmunir            #+#    #+#             */
/*   Updated: 2024/08/08 15:37:10 by abashir          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

int main(int ac, const char **av)
{
    try
    {
        if (ac > 2)
            throw std::runtime_error("Usage: ./webserv <config_file>");
        if (ac == 1)
            av[1] = "config2.conf";
        Parser parser(av[1]);
        // printServers(parser.getServers());
        // printports
        std::vector<std::string> ports = parser.getPorts();
        // for (std::vector<std::string>::iterator it = ports.begin(); it != ports.end(); it++)
        //     std::cout << *it << std::endl;
        Validation validation(parser);
        Server server(parser);
        server.run(parser);
    }
    catch (std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    return 0;
}
