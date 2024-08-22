#include "ServerManager.hpp"

ServerManager::ServerManager(Parser &parser)
{
	std::vector<ServerConfig> serverConfigs = parser.getServers();
	for (std::vector<ServerConfig>::iterator it = serverConfigs.begin(); it != serverConfigs.end(); it++)
	{
		Server* server = new Server(*it, kqueue);
		server->run();
		if (server->serverError == -1)
		{
			delete server;
			continue;
		}
		this->servers.push_back(server);
		this->serverSockets.push_back(server->getServerSocket());
	}

	if (this->servers.size() == 0)
		throw std::runtime_error("No server was created");

	for (size_t i = 0; i < this->serverSockets.size(); i++)
		this->kqueue.addToQueue(this->serverSockets[i], READ_EVENT);
	
	while (true)
	{
		int numOfEvents = this->kqueue.getNumOfEvents();
		for (int i = 0; i < numOfEvents; i++)
		{
			struct kevent event = this->kqueue.events[i];
			if (event.filter == EVFILT_READ)
			{
				for (size_t j = 0; j < this->servers.size(); j++)
				{
					if (this->servers[j]->getServerSocket() == event.ident)
					{
						this->servers[j]->acceptClient();
						break;
					}
					else
					{
						if (servers[j]->isMyClient(event.ident))
						{
							servers[j]->handleRead(event.ident);
							break;
						}
					}
				}
				
			}
			else if (event.filter == EVFILT_WRITE)
			{
				for (size_t j = 0; j < this->servers.size(); j++)
				{
					if (servers[j]->isMyClient(event.ident))
					{
						servers[j]->handleWrite(event.ident);
						break;
					}
				}
			}

		}
	}
}

ServerManager::~ServerManager()
{
	for (std::vector<Server *>::iterator it = this->servers.begin(); it != this->servers.end(); it++)
	{
		delete *it;
	}
}
