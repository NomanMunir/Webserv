#include "ServerConfig.hpp"

int main() {
    
    try
    {
        Http http("config.conf");
        std::vector<ServerConfig> servers = http.getServers();
        for (size_t i = 0; i < servers.size(); i++)
        {
            std::cout << "Host: " << servers[i].host << std::endl;
            std::cout << "Port: " << servers[i].port << std::endl;
            std::cout << "Server Name: " << servers[i].serverName << std::endl;
            std::cout << "Error Pages: " << std::endl;
            for (auto it = servers[i].errorPages.begin(); it != servers[i].errorPages.end(); it++)
            {
                std::cout << it->first << " " << it->second << std::endl;
            }
            std::cout << "Client Body Size Limit: " << servers[i].clientBodySizeLimit << std::endl;
            std::cout << "Routes: " << std::endl;
            for (auto it = servers[i].routes.begin(); it != servers[i].routes.end(); it++)
            {
                std::cout << "Path: " << it->first << std::endl;
                std::cout << "Methods: ";
                for (size_t j = 0; j < it->second.methods.size(); j++)
                {
                    std::cout << it->second.methods[j] << " ";
                }
                std::cout << std::endl;
                std::cout << "Redirect: " << it->second.redirect << std::endl;
                std::cout << "Root: " << it->second.root << std::endl;
                std::cout << "Directory Listing: " << it->second.directoryListing << std::endl;
                std::cout << "Default File: " << it->second.defaultFile << std::endl;
                std::cout << "CGI Path: " << it->second.cgiPath << std::endl;
                std::cout << "Upload Dir: " << it->second.uploadDir << std::endl;
            }
        }   
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    return 0;
}
