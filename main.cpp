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

#include "events/EventPoller.hpp"

#if defined(__APPLE__) || defined(__FreeBSD__)
    #include "events/KQueuePoller.hpp"
#elif defined(__linux__)
    #include "events/EpollPoller.hpp"
#endif

#include "ServerManager.hpp"
#include <csignal>
#include <stdexcept>
#include "utils/Logs.hpp"
#include <unistd.h>
#include <sys/wait.h>
#include "utils/MimeTypes.hpp"

void handleSignal(int signal)
{
    if (signal == SIGINT || signal == SIGTERM)
    {
        std::cout << "\nShutting down server..." << std::endl;
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

    Logs::appendLog("INFO", "[initializeSignalHandling]\t\t Initializing signal handling");
    // Clear the sigaction struct
    std::memset(&sa, 0, sizeof(sa));
    signal(SIGPIPE, SIG_IGN);
	signal(SIGINT, handleSignal);
	signal(SIGTERM, handleSignal);
    signal(SIGCHLD, handleSignal);

    sa.sa_handler = handleSignal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;

    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGCHLD, &sa, NULL);
}

int main(int ac, const char **av, char **env)
{
    EventPoller *poller;
    try
    {
        Logs::init();
        
        if (ac > 2)
            throw std::runtime_error("[main]\t\t\t Usage: ./webserv <config_file>");
        if (ac == 1)
            av[1] = "config/config.conf";

        initializeSignalHandling();
        MimeTypes::load("config/mime.types");
        Parser parser(av[1]);

        Validation validation(parser);
        parser.setEnv(env);

        #if defined(__APPLE__) || defined(__FreeBSD__)
            poller = new KQueuePoller();
        #elif defined(__linux__)
            poller = new EpollPoller();
        #else
            throw std::runtime_error("[main]\t\t\t Unsupported OS");
        #endif

        ServerManager serverManager(parser, poller);
        serverManager.run();
    }
    catch (std::exception &e)
    {
        Logs::appendLog("ERROR", e.what());
        Logs::close();
        std::cerr << e.what() << std::endl;
        return 1;
    }
    return 0;
}
