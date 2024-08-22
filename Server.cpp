#include "Server.hpp"

#if defined(__APPLE__) || defined(__FreeBSD__)
#include "events/KQueue.hpp"
#endif // __APPLE__ || __FreeBSD__

#if defined(__linux__)
#include "events/EPoll.hpp"
#endif // __linux__

#define PORT 80
#define MAX_CONNECTIONS 10

Server::Server(ServerConfig &serverConfig) :serverConfig(serverConfig), serverError(0)
{
    std::string serverPort = serverConfig.listen.back().back();
    this->port =  std::atoi(serverPort.c_str());
    std::memset((char *)&addr, 0, sizeof(addr));
}

Server::~Server()
{
    close(this->serverSocket);
}

Server::Server(const Server &other)
{
    this->serverSocket = other.serverSocket;
    this->addr = other.addr;
    this->port = other.port;
    this->serverError = other.serverError;
    this->serverConfig = other.serverConfig;
}

Server &Server::operator=(const Server &other)
{
    if (this != &other)
    {
        this->serverSocket = other.serverSocket;
        this->addr = other.addr;
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
        throw std::runtime_error("Error setsockopt option");
}

void Server::bindAndListen()
{
   this->addr.sin_family = AF_INET;
   this->addr.sin_addr.s_addr = INADDR_ANY;
   this->addr.sin_port = htons(this->port);
    
    if (bind(this->serverSocket, (struct sockaddr *)&addr, sizeof(addr)) < 0)
       throw std::runtime_error("Error binding socket");
    if (listen(this->serverSocket, MAX_CONNECTIONS) < 0)
        throw std::runtime_error("Error listening on socket");

    std::cout << "Server is running on port " << this->port << std::endl;
}

void Server::initSocket()
{
    this->serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    std::cout<< "serverSocket: " << this->serverSocket << std::endl;
    if (this->serverSocket < 0)
        throw std::runtime_error("Error opening socket");
}

void Server::run()
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
        if (this->serverSocket > 0)
            close(this->serverSocket);
        this->serverError = -1;
    }
    
}

int Server::getPort() const { return this->port; }

int Server::getServerSocket() const { return this->serverSocket; }

struct sockaddr_in Server::getAddr() const { return this->addr; }

ServerConfig Server::getServerConfig() const { return this->serverConfig; }

// void Server::initServerSockets(Parser &configFile) 
// {
//     for (std::vector<std::string>::iterator it = ports.begin(); it != ports.end(); ++it) {
//         int port = std::atoi((*it).c_str());
//         int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
//         if (serverSocket < 0) {
//             perror("Error opening socket");
//             throw std::runtime_error("Error opening socket");
//         }
//         setNonBlocking(serverSocket);
//         int e = 1;
//         if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&e, sizeof(e)) < 0) {
//             perror("Error setsockopt option");
//             close(serverSocket);
//             throw std::runtime_error("Error setsockopt option");
//         }

//         struct sockaddr_in serverAddr;
//         std::memset((char *)&serverAddr, 0, sizeof(serverAddr));
//         serverAddr.sin_family = AF_INET;
//         serverAddr.sin_addr.s_addr = INADDR_ANY;
//         serverAddr.sin_port = htons(port);

//         if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
//             perror("Error binding socket");
//             close(serverSocket);
//             throw std::runtime_error("Error binding socket");
//         }

//         if (listen(serverSocket, 3) < 0) {
//             perror("Error listening on socket");
//             close(serverSocket);
//             throw std::runtime_error("Error listening on socket");
//         }

//         serverSockets.push_back(serverSocket);
//         serverAddrs.push_back(serverAddr);
//         std::cout << "Server is running on port " << port << std::endl;
//     }
// }

// void Server::handleConnections(Parser &configFile)
// {
//     Connections connections(this->serverSockets, configFile);
//     connections.loop();
// }

