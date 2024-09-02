#ifndef SERVERMANAGER_HPP
#define SERVERMANAGER_HPP

#include "Server.hpp"
#include <vector>
#include "parsing/Parser.hpp"
#include "events/KQueue.hpp"

class ServerManager
{
	public:
		ServerManager(Parser &parser);
		~ServerManager();
		// ServerManager(const ServerManager &other);
		// ServerManager &operator=(const ServerManager &other);
		static bool running;
		void run();

	private:
		std::vector<Server *> servers;
		std::vector<int> serverSockets;
		KQueue kqueue;

		void processReadEvent(EventInfo eventInfo);
		void processWriteEvent(EventInfo eventInfo);
		void processTimeoutEvent(EventInfo eventInfo);

		void stop();

		void initServers(Parser &parser);
};

#endif // SERVERMANAGER_HPP
