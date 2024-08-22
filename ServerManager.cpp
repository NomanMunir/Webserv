#include "ServerManager.hpp"



ServerManager::ServerManager(Parser &parser)
{
	std::vector<ServerConfig> serverConfigs = parser.getServers();
	for (std::vector<ServerConfig>::iterator it = serverConfigs.begin(); it != serverConfigs.end(); it++)
	{
		Server server(*it);
		this->servers.push_back(server);
		this->servers.back().run();
		if (this->servers.back().serverError == -1)
			continue;
		this->serverSockets.push_back(this->servers.back().getServerSocket());
	}
	if (this->servers.size() == 0)
		throw std::runtime_error("No server was created");
	Connections connections(this->serverSockets, parser);
	connections.loop();
}

