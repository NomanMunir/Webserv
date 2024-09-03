#include "Server.hpp"

#if defined(__APPLE__) || defined(__FreeBSD__)
#include "events/EventPoller.hpp"
#endif // __APPLE__ || __FreeBSD__

#if defined(__linux__)
#include "events/EPoll.hpp"
#endif // __linux__

#define MAX_CONNECTIONS 10


Server::Server(ServerConfig &serverConfig, EventPoller *poller) 
    :serverConfig(serverConfig), _poller(poller), serverError(0)
{
    std::string serverPort = serverConfig.listen.back().back();
    this->port =  std::atoi(serverPort.c_str());
    std::memset((char *)&addr, 0, sizeof(addr));
}

Server::~Server() { close(this->serverSocket); }

Server::Server(const Server &other) 
    : serverConfig(other.serverConfig),
     _poller(other._poller), serverError(other.serverError),
      port(other.port), addr(other.addr), serverSocket(other.serverSocket) {}

Server &Server::operator=(const Server &other)
{
    if (this != &other)
    {
        this->serverSocket = other.serverSocket;
        this->addr = other.addr;
        this->_poller = other._poller;
        this->port = other.port;
        this->serverError = other.serverError;
        this->serverConfig = other.serverConfig;
    }
    return *this;
}

void Server::socketInUse()
{
    int e = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&e, sizeof(e)) < 0)
        throw std::runtime_error("[socketInUse]\t\t Error setting socket options " + std::string(strerror(errno)));
}

void Server::bindAndListen()
{
    this->addr.sin_family = AF_INET;
    std::string ipAddress = this->serverConfig.listen.back().front();
    this->addr.sin_port = htons(this->port);

    if (inet_pton(AF_INET, ipAddress.c_str(), &this->addr.sin_addr) <= 0)
        throw std::runtime_error("[bindAndListen]\t\t Invalid address/ Address not supported " + std::string(strerror(errno)));

    if (bind(this->serverSocket, (struct sockaddr *)&addr, sizeof(addr)) < 0)
       throw std::runtime_error("[bindAndListen]\t\t Error binding socket " + std::string(strerror(errno)));

    if (listen(this->serverSocket, MAX_CONNECTIONS) < 0)
        throw std::runtime_error("[bindAndListen]\t\t Error listening on socket " + std::string(strerror(errno)));

    std::cout << "Server is listening at http://" << inet_ntoa(addr.sin_addr) << ":" << this->port << std::endl;
    Logs::appendLog("INFO", "[bindAndListen]\t\t Server is listening at http://" + ipAddress + ":" + std::to_string(this->port));
}

void Server::initSocket()
{
    this->serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (this->serverSocket < 0)
        throw std::runtime_error("[initSocket]\t\t Error opening socket " + std::string(strerror(errno)));
}

void Server::init()
{
    try
    {
        initSocket();
        socketInUse();
        bindAndListen();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        Logs::appendLog("ERROR", e.what());
        if (this->serverSocket > 0)
            close(this->serverSocket);
        this->serverError = -1;
    }
    
}

bool Server::acceptClient()
{
    struct sockaddr_in clientAddr;
    socklen_t clientLen = sizeof(clientAddr);
    int clientSocket = accept(this->serverSocket, (struct sockaddr *)&clientAddr, &clientLen);
    if (clientSocket < 0)
    {
        Logs::appendLog("ERROR", "[acceptClient]\t\t Error accepting client " + std::string(strerror(errno)));
        return false;
    }
    Logs::appendLog("INFO", "[acceptClient]\t\t New connection, socket fd is [" + std::to_string(clientSocket) + "], IP is : " + inet_ntoa(clientAddr.sin_addr) + ", port : " + std::to_string(ntohs(clientAddr.sin_port)));
    clients[clientSocket] = Client(clientSocket);
    this->_poller->addToQueue(clientSocket, READ_EVENT);
    this->_poller->addToQueue(clientSocket, TIMEOUT_EVENT);
    return true;
}


bool Server::isMyClient(int fd)
{
    return clients.find(fd) != clients.end();
}

void Server::handleWrite(int fd)
{
    Logs::appendLog("INFO", "[handleWrite]\t\t Handling write for client " + std::to_string(fd));
    try
    {
        bool isClosed = !clients[fd].isKeepAlive() || clients[fd].getResponse().getIsConnectionClosed();
        if (isClosed)
        {
            ssize_t bytesSent = send(fd, clients[fd].getWriteBuffer().c_str(), clients[fd].getWriteBuffer().size(), 0);
            Logs::appendLog("INFO", "[handleWrite]\t\t Connection closed for client [" + std::to_string(fd) + "] with IP " + inet_ntoa(this->addr.sin_addr) + " on port " + std::to_string(this->port));
            this->_poller->removeFromQueue(fd, WRITE_EVENT);
            this->_poller->removeFromQueue(fd, TIMEOUT_EVENT);
            this->_poller->removeFromQueue(fd, READ_EVENT);
            close(fd);
            clients.erase(fd);
            return;
        }
        if (!clients[fd].isWritePending()) return;

        ssize_t bytesSent = send(fd, clients[fd].getWriteBuffer().c_str(), clients[fd].getWriteBuffer().size(), 0);
        if (bytesSent < 0)
        {
            Logs::appendLog("ERROR", "[handleWrite]\t\t Error sending data to client " + std::string(strerror(errno)));
            this->_poller->removeFromQueue(fd, WRITE_EVENT);
            return;
        }
        
        this->_poller->removeFromQueue(fd, WRITE_EVENT);
        clients[fd].getWriteBuffer().clear();
        clients[fd].setWritePending(false);

        clients[fd].reset();
        this->_poller->addToQueue(fd, TIMEOUT_EVENT);

    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        this->_poller->removeFromQueue(fd, TIMEOUT_EVENT);
        close(fd);
        clients.erase(fd);
    }
}

void Server::handleRead(int fd)
{
    try
    {
        clients[fd].readFromSocket(this->serverConfig);
        if (clients[fd].getResponse().getIsConnectionClosed() || !clients[fd].isKeepAlive())
            this->_poller->removeFromQueue(fd, READ_EVENT);

        if (clients[fd].isWritePending())
            this->_poller->addToQueue(fd, WRITE_EVENT);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        this->_poller->removeFromQueue(fd, READ_EVENT);
        this->_poller->removeFromQueue(fd, TIMEOUT_EVENT);
        close(fd);
        clients.erase(fd);
    }
}

void Server::handleDisconnection(int fd)
{
    Logs::appendLog("DEBUG", "[handleDisconnection]\t\t Handling disconnection for client " + std::to_string(fd));
    this->_poller->removeFromQueue(fd, READ_EVENT);
    this->_poller->removeFromQueue(fd, TIMEOUT_EVENT);
    close(fd);
    clients.erase(fd);
}


int Server::getPort() const { return this->port; }

std::unordered_map<int, Client> &Server::getClients() { return this->clients; }

int Server::getServerSocket() const { return this->serverSocket; }

struct sockaddr_in Server::getAddr() const { return this->addr; }

ServerConfig Server::getServerConfig() const { return this->serverConfig; }
