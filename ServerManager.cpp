#include "ServerManager.hpp"

bool ServerManager::running = true;

ServerManager::~ServerManager()
{
	for (std::vector<Server *>::iterator it = this->servers.begin(); it != this->servers.end(); it++)
	{
		delete *it;
	}
}

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

void ServerManager::processReadEvent(EventInfo eventInfo)
{
	for (size_t j = 0; j < this->servers.size(); j++)
	{
		if (this->servers[j]->getServerSocket() == eventInfo.fd)
		{
			if (!this->servers[j]->acceptClient())
				continue;
			break;
		}
		else
		{
			if (this->servers[j]->isMyClient(eventInfo.fd))
			{
				this->servers[j]->handleRead(eventInfo.fd);
				break;
			}
		}
	}
}

void ServerManager::processWriteEvent(EventInfo eventInfo)
{
	for (size_t j = 0; j < this->servers.size(); j++)
	{
		if (servers[j]->isMyClient(eventInfo.fd))
		{
			servers[j]->handleWrite(eventInfo.fd);
			break;
		}
	}
}

void ServerManager::processTimeoutEvent(EventInfo eventInfo)
{
	for (size_t j = 0; j < this->servers.size(); j++)
	{
		if (servers[j]->isMyClient(eventInfo.fd))
		{
			servers[j]->handleDisconnection(eventInfo.fd);
			break;
		}
	}
}

void ServerManager::run()
{
	while (ServerManager::running)
	{
		int numOfEvents = this->kqueue.getNumOfEvents();
		if (running == false)
			break;
		if (numOfEvents < 0)
		{
			std::cerr << "run1::Error: " << strerror(errno) << std::endl;
			continue;
		}
		if (numOfEvents == 0)
            continue;

		for (int i = 0; i < numOfEvents; i++)
		{
			EventInfo eventInfo = this->kqueue.getEventInfo(i);
			if (eventInfo.isError)
			{
				std::cerr << "run::Error: " << strerror(errno) << std::endl;
				continue;
			}
			if (eventInfo.isRead || eventInfo.isEOF)
				processReadEvent(eventInfo);
		}

		for (size_t i = 0; i < numOfEvents; i++)
		{
			EventInfo eventInfo = this->kqueue.getEventInfo(i);
			if (eventInfo.isWrite)
				processWriteEvent(eventInfo);
		}

		for (size_t i = 0; i < numOfEvents; i++)
		{
			EventInfo eventInfo = this->kqueue.getEventInfo(i);
			if (eventInfo.isTimeout)
				processTimeoutEvent(eventInfo);
		}
	}
}
