#ifndef SERVERMANAGER_HPP
#define SERVERMANAGER_HPP

#include "Server.hpp"
#include <vector>
#include "parsing/Parser.hpp"
#include "events/EventPoller.hpp"
#include "utils/Logs.hpp"

class ServerManager
{
	public:
		ServerManager(Parser &parser, EventPoller *poller);
		~ServerManager();
		// ServerManager(const ServerManager &other);
		// ServerManager &operator=(const ServerManager &other);
		static bool running;
		void run();

	private:
		std::vector<Server *> servers;
		std::vector<int> serverSockets;
		EventPoller *_poller;

		void processReadEvent(EventInfo eventInfo);
		void processWriteEvent(EventInfo eventInfo);
		void processTimeoutEvent(EventInfo eventInfo);

		void initServers(Parser &parser);
};

#endif // SERVERMANAGER_HPP
