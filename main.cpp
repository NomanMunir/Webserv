/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: absalem < absalem@student.42abudhabi.ae    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/08 17:07:42 by nmunir            #+#    #+#             */
/*   Updated: 2024/08/15 13:20:50 by absalem          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerManager.hpp"
#include <csignal>
#include <stdexcept>

void handleSignal(int signal)
{
    if (signal == SIGINT || signal == SIGTERM)
    {
        ServerManager::running = false;
    }
    else if (signal == SIGCHLD)
	{
		int status;
		pid_t pid;
		while ((pid = waitpid(-1, &status, WNOHANG)) > 0) { }
	}
}

void initializeSignalHandling()
{
    struct sigaction sa;

    // Clear the sigaction struct
    std::memset(&sa, 0, sizeof(sa));
    signal(SIGPIPE, SIG_IGN);
	signal(SIGINT, handleSignal);
	signal(SIGTERM, handleSignal);
    // Set the signal handler function for SIGINT and SIGTERM
    sa.sa_handler = handleSignal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP; // Ensure that interrupted system calls are restarted

    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGCHLD, &sa, NULL);
}

int main(int ac, const char **av, char **env)
{
    try
    {
        if (ac > 2)
            throw std::runtime_error("Usage: ./webserv <config_file>");
        if (ac == 1)
            av[1] = "config1.conf";
        initializeSignalHandling();
        Parser parser(av[1]);

        Validation validation(parser);
        parser.setEnv(env);
        ServerManager serverManager(parser);
        serverManager.run();
    }
    catch (std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    return 0;
}
