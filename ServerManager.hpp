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
		// ~ServerManager();
		// ServerManager(const ServerManager &other);
		// ServerManager &operator=(const ServerManager &other);
	
	private:
		std::vector<Server> servers;
		std::vector<int> serverSockets;


		
};

#endif // SERVERMANAGER_HPP
