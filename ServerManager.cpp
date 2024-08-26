#include "ServerManager.hpp"

void ServerManager::initServers(Parser &parser)
{
	std::vector<ServerConfig> serverConfigs = parser.getServers();
	for (std::vector<ServerConfig>::iterator it = serverConfigs.begin(); it != serverConfigs.end(); it++)
	{
		Server* server = new Server(*it, kqueue);
		server->init();
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
}

ServerManager::ServerManager(Parser &parser)
{
	initServers(parser);
}

void ServerManager::run()
{
	while (true)
	{
		int numOfEvents = this->kqueue.getNumOfEvents();
		if (numOfEvents == -1)
		{
			std::cerr << "Error: " << strerror(errno) << std::endl;
			continue;
		}
		if (numOfEvents == 0)
            continue;

		for (int i = 0; i < numOfEvents; i++)
		{
			struct kevent event = this->kqueue.events[i];
			if (event.filter == EVFILT_READ)
			{
				for (size_t j = 0; j < this->servers.size(); j++)
				{
					if (this->servers[j]->getServerSocket() == event.ident)
					{
						if (!this->servers[j]->acceptClient())
							continue;
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
		}

		for (size_t i = 0; i < numOfEvents; i++)
		{
			struct kevent event = this->kqueue.events[i];
			if (event.flags & EV_ERROR)
            {
                std::cerr << "EV_ERROR: " << strerror(event.data) << std::endl;
                continue;
            }
			if (event.filter == EVFILT_WRITE)
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
		for (size_t i = 0; i < numOfEvents; i++)
		{
			struct kevent event = this->kqueue.events[i];
			if (event.filter == EVFILT_TIMER)
			{
				for (size_t j = 0; j < this->servers.size(); j++)
				{
					if (servers[j]->isMyClient(event.ident))
					{
						servers[j]->handleTimeout(event.ident);
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
