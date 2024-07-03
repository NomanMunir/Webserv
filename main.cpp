/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmunir <nmunir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/08 17:07:42 by nmunir            #+#    #+#             */
/*   Updated: 2024/07/03 10:26:40 by nmunir           ###   ########.fr       */
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
            av[1] = "config1.conf";
        Parser parser(av[1]);
        Validation validation(parser);
        printServers(parser.getServers());
        // Server server(parser);
        // server.run();
    }
    catch (std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    return 0;
}
// int main() {
//     std::string request = " GET http://www.example.com HTTP/1.1\r\nHost:example.com\r\nConnection: keep-alive\r\n\r\n";
//     try {
//         Headers headers(request);
//     } catch (const std::exception& e)
//     {
//         std::cerr << "Error: " << e.what() << std::endl;
//     }
//     // GET / HTTP/1.1
//     return 0;
// }