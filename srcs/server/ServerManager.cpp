#include "ServerManager.hpp"

bool ServerManager::running = true;

ServerManager::ServerManager(Parser &parser, EventPoller *poller) : _poller(poller)
{
	initServers(parser);
}

ServerManager::~ServerManager()
{
	for (std::vector<Server *>::iterator it = this->servers.begin(); it != this->servers.end(); it++)
	{
		delete *it;
	}
	delete this->_poller;
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
		server->setServerManager(this);
		this->servers.push_back(server);
		int serverSocket = server->getServerSocket();
		this->serverSockets.push_back(serverSocket);
		this->serverSocketMap[serverSocket] = server;
	}

	if (this->servers.size() == 0)
	{
		delete this->_poller;
		throw std::runtime_error("[initServers]\t\t Unable to create any server");
	}

	for (size_t i = 0; i < this->serverSockets.size(); i++)
		this->_poller->addToQueue(this->serverSockets[i], READ_EVENT);
}

void ServerManager::processReadEvent(EventInfo eventInfo)
{
	// O(1) lookup: Check if this is a server socket
	std::map<int, Server*>::iterator serverIt = serverSocketMap.find(eventInfo.fd);
	if (serverIt != serverSocketMap.end())
	{
		// This is a new connection on a server socket
		serverIt->second->acceptClient();
		return;
	}
	
	// O(1) lookup: Find which server owns this client fd
	Server* server = getServerForFd(eventInfo.fd);
	if (server != NULL)
	{
		// Handle client disconnect
		if (eventInfo.isEOF || eventInfo.isError)
		{
			server->handleDisconnection(eventInfo.fd);
			unregisterClientFd(eventInfo.fd);
			return;
		}
		
		// Handle normal read
		server->handleRead(eventInfo.fd);
		return;
	}
	
	// If not found in client map, check if it's a CGI fd
	// CGI fds are not pre-registered, so we need to check all servers
	for (size_t j = 0; j < this->servers.size(); j++)
	{
		if (this->servers[j]->isMyCGI(eventInfo.fd))
			return; // CGI handler already processed
	}
}

void ServerManager::processWriteEvent(EventInfo eventInfo)
{
	// O(1) lookup: Find which server owns this client fd
	Server* server = getServerForFd(eventInfo.fd);
	if (server == NULL)
		return; // Unknown fd, ignore
	
	server->handleWrite(eventInfo.fd);
}

void ServerManager::processTimeoutEvent(EventInfo eventInfo)
{
	// O(1) lookup: Find which server owns this client fd
	Server* server = getServerForFd(eventInfo.fd);
	if (server == NULL)
		return; // Unknown fd, ignore
	
	server->handleDisconnection(eventInfo.fd);
	unregisterClientFd(eventInfo.fd);
}

void ServerManager::checkTimeouts()
{
	for (size_t i = 0; i < this->servers.size(); i++)
		this->servers[i]->checkTimeouts();
}
void ServerManager::run()
{
	while (ServerManager::running)
	{
		checkTimeouts();
		int numOfEvents = this->_poller->getNumOfEvents();
		if (running == false)
			break;
		if (numOfEvents < 0)
		{
			Logs::appendLog("ERROR", "[run]\t\t Number of events is invalid");
			continue;
		}
		if (numOfEvents == 0)
            continue;

		for (int i = 0; i < numOfEvents; i++)
		{
			EventInfo eventInfo = this->_poller->getEventInfo(i);
			if (eventInfo.isRead || eventInfo.isEOF || eventInfo.isError)
				processReadEvent(eventInfo);
			else if (eventInfo.isWrite)
				processWriteEvent(eventInfo);
		}
	}
}

void ServerManager::registerClientFd(int fd, Server* server)
{
	fdToServer[fd] = server;
}

void ServerManager::unregisterClientFd(int fd)
{
	fdToServer.erase(fd);
}

Server* ServerManager::getServerForFd(int fd)
{
	std::map<int, Server*>::iterator it = fdToServer.find(fd);
	if (it != fdToServer.end())
		return it->second;
	return NULL;
}
