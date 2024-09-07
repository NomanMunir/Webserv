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
		Server* server = new Server(*it, this->_poller);
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
		throw std::runtime_error("[initServers]\t\t Unable to create any server");

	for (size_t i = 0; i < this->serverSockets.size(); i++)
		this->_poller->addToQueue(this->serverSockets[i], READ_EVENT);
}

ServerManager::ServerManager(Parser &parser, EventPoller *poller) : _poller(poller)
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
		int numOfEvents = this->_poller->getNumOfEvents();
		if (running == false)
			break;
		if (numOfEvents < 0)
		{
			Logs::appendLog("ERROR", "[run]\t\t Number of events is invalid");
			continue;
		}
		if (numOfEvents == 0)
		{
			Logs::appendLog("DEBUG", "[run]\t\t No events to process");
            continue;
		}

		for (int i = 0; i < numOfEvents; i++)
		{
			EventInfo eventInfo = this->_poller->getEventInfo(i);
			if (eventInfo.isError)
			{
				Logs::appendLog("ERROR", "[run]\t\t Error event at fd: [" + std::to_string(eventInfo.fd) + "]");
				continue;
			}
			if (eventInfo.isRead || eventInfo.isEOF)
				processReadEvent(eventInfo);
		}

		for (size_t i = 0; i < numOfEvents; i++)
		{
			EventInfo eventInfo = this->_poller->getEventInfo(i);
			if (eventInfo.isWrite)
				processWriteEvent(eventInfo);
		}

		// for (size_t i = 0; i < numOfEvents; i++)
		// {
		// 	EventInfo eventInfo = this->_poller->getEventInfo(i);
		// 	if (eventInfo.isTimeout)
		// 		processTimeoutEvent(eventInfo);
		// }
	}
}
