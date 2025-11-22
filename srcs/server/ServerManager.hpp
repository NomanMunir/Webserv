#ifndef SERVERMANAGER_HPP
#define SERVERMANAGER_HPP

#include "Server.hpp"
#include <vector>
#include <map>
#include "../parsing/Parser.hpp"
#include "../events/EventPoller.hpp"
#include "../utils/Logs.hpp"

class ServerManager
{
	public:
		ServerManager(Parser &parser, EventPoller *poller);
		~ServerManager();
		static bool running;
		void run();
		
		// Allow Server to register/unregister client fds
		void registerClientFd(int fd, Server* server);
		void unregisterClientFd(int fd);

	private:
		std::vector<Server *> servers;
		std::vector<int> serverSockets;
		EventPoller *_poller;
		
		// O(1) lookup maps for efficient event dispatching
		std::map<int, Server*> fdToServer;        // Maps any fd (client/CGI) to owning server
		std::map<int, Server*> serverSocketMap;   // Maps server socket to server

		void processReadEvent(EventInfo eventInfo);
		void processWriteEvent(EventInfo eventInfo);
		void processTimeoutEvent(EventInfo eventInfo);

		void checkTimeouts();
		void initServers(Parser &parser);
		
		Server* getServerForFd(int fd);
};

#endif // SERVERMANAGER_HPP
