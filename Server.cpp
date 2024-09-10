#include "Server.hpp"
#include "events/EventPoller.hpp"

#define MAX_CONNECTIONS 10


Server::Server(ServerConfig &serverConfig, EventPoller *poller) 
    :serverConfig(serverConfig), _poller(poller), serverError(0)
{
    std::string serverPort = serverConfig.listen.back().back();
    this->port =  std::atoi(serverPort.c_str());
    std::memset((char *)&addr, 0, sizeof(addr));
}

Server::~Server()
{
    close(this->serverSocket);
    while (!clients.empty())
    {
        close(clients.begin()->first);
        // if (clients.begin()->second.isCGI())
            // clients.begin()->second.getCgi().killCGI();
        clients.erase(clients.begin());
    }
}

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
    int flags = fcntl(clientSocket, F_GETFL, 0);
    if (flags == -1)
    {
        Logs::appendLog("ERROR", "[acceptClient]\t\t " + std::string(strerror(errno)));
        close(clientSocket);
        return false;
    }
    if (fcntl(clientSocket, F_SETFL, flags | O_NONBLOCK) == -1)
    {
        Logs::appendLog("ERROR", "[acceptClient]\t\t " + std::string(strerror(errno)));
        close(clientSocket);
        return false;
    }

    Logs::appendLog("INFO", "[acceptClient]\t\t New connection, socket fd is [" + std::to_string(clientSocket) + "], IP is : " + inet_ntoa(clientAddr.sin_addr) + ", port : " + std::to_string(ntohs(clientAddr.sin_port)));
    clients[clientSocket] = Client(clientSocket, this->_poller);
    this->_poller->addToQueue(clientSocket, READ_EVENT);
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
        clients[fd].updateLastActivity();
        Logs::appendLog("INFO", "[handleWrite]\t\t Write completed for client " + std::to_string(fd));
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
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
        // this->_poller->removeFromQueue(fd, TIMEOUT_EVENT);
        close(fd);
        clients.erase(fd);
    }
}


bool Server::isMyCGI(int fd)
{
    std::unordered_map<int, Client>::iterator it = clients.begin();
    while (it != this->clients.end())
    {
        if (it->second.getCgi().getReadFd() == fd)
        {
            handleCgiRead(it->first);
            return true;
        }
        ++it;
    }
    return false;
}

void Server::handleCgiRead(int clientFd)
{
    ssize_t count;
    char buffer[1024];

    Client &client = this->clients[clientFd];
    
    Logs::appendLog("INFO", "[handleCgiRead]\t\t Reading from CGI process " + std::to_string(client.getCgi().getPid()));
    while ((count = read(client.getCgi().getReadFd(), buffer, sizeof(buffer))) > 0)
        client.getCgi().output.append(buffer, count);

    if (count == 0)
    {
        this->_poller->removeFromQueue(client.getCgi().getReadFd(), READ_EVENT);
        close(client.getCgi().getReadFd());

        HttpResponse httpResponse;
        httpResponse.setVersion("HTTP/1.1");
        httpResponse.setStatusCode(200);
        httpResponse.setHeader("Content-Type", "text/html");
        httpResponse.setHeader("Content-Length", std::to_string(client.getCgi().output.size()));
        httpResponse.setHeader("Connection", "keep-alive");

        std::string cookies = client.getRequest().getHeaders().getValue("Cookie");
        if (!cookies.empty())
            httpResponse.setHeader("Set-Cookie", cookies);

        httpResponse.setHeader("Server", "LULUGINX");
        httpResponse.setBody(client.getCgi().output);

        client.getWriteBuffer() = httpResponse.generateResponse();
        client.setWritePending(true);
        client.setReadPending(false);

        this->_poller->addToQueue(clientFd, WRITE_EVENT);
    }
    else if (count < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            Logs::appendLog("DEBUG", "[handleCgiRead]\t\t CGI process " + std::to_string(client.getCgi().getPid()) + " is still running and waiting for more data");
            return;
        }
        else
        {
            Logs::appendLog("ERROR", "[handleCgiRead]\t\t Error reading from CGI " + std::string(strerror(errno)));
            kill(client.getCgi().getPid(), SIGKILL);
            Logs::appendLog("INFO", "[checkTimeouts]\t\t Killed CGI process " + std::to_string(client.getCgi().getPid()));
            this->_poller->removeFromQueue(clientFd, READ_EVENT);
            this->_poller->removeFromQueue(client.getCgi().getReadFd(), READ_EVENT);
            generateCGIError(client);
            this->_poller->addToQueue(clientFd, WRITE_EVENT);
            client.getResponse().setIsConnectionClosed(true);
        }
    }
}

void Server::generateCGIError(Client &client)
{
    HttpResponse httpResponse;
    httpResponse.setVersion("HTTP/1.1");
    httpResponse.setStatusCode(504);
    httpResponse.setHeader("Content-Type", "text/html");
    httpResponse.setHeader("Connection", "close");
    httpResponse.setHeader("Server", "LULUGINX");
    std::string body = "<html><body><h1>504 Gateway Timeout</h1></body></html>";
    httpResponse.setBody(body);
    httpResponse.setHeader("Content-Length", std::to_string(body.size()));
    client.getWriteBuffer() = httpResponse.generateResponse();
    client.setWritePending(true);
}

void Server::checkTimeouts()
{
    std::unordered_map<int, Client>::iterator it = clients.begin();

    while (it != this->clients.end())
    {
        if (it->second.isTimeOut())
        {
            if (it->second.getRequest().getIsCGI())
            {
                Logs::appendLog("INFO", "[checkTimeouts]\t\t CGI process for client " + std::to_string(it->first) + " timed out");
                kill(it->second.getCgi().getPid(), SIGKILL);
                Logs::appendLog("INFO", "[checkTimeouts]\t\t Killed CGI process " + std::to_string(it->second.getCgi().getPid()));
                this->_poller->removeFromQueue(it->first, READ_EVENT);
                this->_poller->removeFromQueue(it->second.getCgi().getReadFd(), READ_EVENT);
                generateCGIError(it->second);
                this->_poller->addToQueue(it->first, WRITE_EVENT);
                it->second.getResponse().setIsConnectionClosed(true);
                ++it;
            }
            else
            {
                Logs::appendLog("INFO", "[checkTimeouts]\t\t Client " + std::to_string(it->first) + " timed out");
                this->_poller->removeFromQueue(it->first, READ_EVENT);
                it = clients.erase(it);
            }
        }
        else
            ++it;
    }
}

void Server::handleDisconnection(int fd)
{
    Logs::appendLog("DEBUG", "[handleDisconnection]\t\t Handling disconnection for client " + std::to_string(fd));
    this->_poller->removeFromQueue(fd, READ_EVENT);
    clients.erase(fd);
}


int Server::getPort() const { return this->port; }

std::unordered_map<int, Client> &Server::getClients() { return this->clients; }

int Server::getServerSocket() const { return this->serverSocket; }

struct sockaddr_in Server::getAddr() const { return this->addr; }

ServerConfig Server::getServerConfig() const { return this->serverConfig; }
