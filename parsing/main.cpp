#include "ServerConfig.hpp"

std::string trim(const std::string &s)
{
    size_t start = s.find_first_not_of(" \t\r\n");
    size_t end = s.find_last_not_of(" \t\r\n");
    return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
}

bool countBrackets(std::vector<std::string> &tokens)
{
    int count = 0;
    for (size_t i = 0; i < tokens.size(); i++)
    {
        if (tokens[i] == "{")
            count++;
        else if (tokens[i] == "}")
            count--;
    }
    return count == 0;
}

void tokanize(std::stringstream &ss, std::vector<std::string> &tokens)
{
    std::string line;
    while (std::getline(ss, line))
    {
        line = trim(line);
        if (line.empty() || line[0] == '#')
            continue;
        std::string token;
        std::stringstream lineStream(line);
        while (lineStream >> token)
            tokens.push_back(token);
    }
}

bool checkHttpDirective(const std::vector<std::string> &tokens, size_t &i)
{
    if (i + 1 >= tokens.size())
        return false;

    const std::string &key = tokens[i];
    const std::string &value = tokens[i + 1];
    std::cout << key << " " << value << std::endl;

    if (key == "client_body_size" || key == "keepalive_timeout")
    {
        if (value[value.size() - 1] == ';')
        {
            size_t limit = key == "keepalive_timeout" ? value.size() - 2 : value.size() - 1;
            for (size_t j = 0; j < limit; j++)
            {
                if (!isdigit(value[j]))
                    return false;
            }
        }
        else
            return false;
    }
    else
        return false;
    i++;
    return true;
}

bool checkLocationBlock(const std::vector<std::string> &tokens, size_t &i)
{
    if (i + 1 >= tokens.size())
        return false;

    const std::string &key = tokens[i];
    std::string value = tokens[i + 1];
    const std::string keys[] = {"path", "methods", "redirect", "root", "directory_listing", "default_file", "cgi_path", "cgi", "upload_dir"};

    while (value.find(";") == std::string::npos)
    {
        if (++i + 1 >= tokens.size())
            return false;
        value += " " + tokens[i + 1];
    }

    if (std::find(keys, keys + 9, key) == keys + 9)
        return false;

    if (value[value.size() - 1] == ';')
    {
        value = value.substr(0, value.size() - 1);
        std::cout << key << " " << value << std::endl;
    }
    else
    {
        return false;
    }

    i++;
    return true;
}

bool checkServerBlock(std::vector<std::string> tokens, size_t &i)
{
    if (i + 1 >= tokens.size())
        return false;
    std::string key = tokens[i];
    std::string value = tokens[i + 1];
    std::string keys[] = {"server_names", "listen", "port", "host", "error_page", "client_body_size", "location"};
    while (value.find(";") == std::string::npos && key != "location")
    {
        if (++i + 1 >= tokens.size())
            return false;
        value += " " + tokens[i + 1];
    }
    if (std::find(std::begin(keys), std::end(keys), key) == std::end(keys))
        return false;
    if (key == "location")
    {
        if (tokens[i + 2] != "{")
            return false;
        std::cout << key << " " << value << std::endl;
        i += 3;
        for (; i < tokens.size() - 1; i++)
        {
            if (tokens[i] == "}")
                return true;
            if (!checkLocationBlock(tokens, i))
                return false;
        }
    }
    else
    {
        if (value[value.size() - 1] == ';')
        {
            value = value.substr(0, value.size() - 1);
            std::cout << key << " " << value << std::endl;
        }
        else
            return false;
    }
    i++;
    return true;
}

bool checkServerDirective(std::vector<std::string> tokens, size_t &i)
{
    for (; i < tokens.size() - 1; i++)
    {
        if (tokens[i] == "}")
            return true;
        if (!checkServerBlock(tokens, i))
            return false;
    }
    return true;
}

bool validateBlocks(std::vector<std::string> &tokens)
{
    bool serverBlock = false;
    if (tokens.empty() || tokens[0] != "http" || tokens[1] != "{")
        return false;
    // tokens.erase(tokens.begin(), tokens.begin() + 2);
    if (tokens[tokens.size() - 1] != "}")
        return false;
    for (size_t i = 2; i < tokens.size() - 1; i++)
    {
        if (tokens[i] == "server")
        {
            serverBlock = true;
            if (tokens[i + 1] != "{")
                return false;
            i += 2;
            if (!checkServerDirective(tokens, i))
                return false;
        }
        else if (tokens[i] == "}")
        {
            if (serverBlock)
                serverBlock = false;
            else
                return false;
        }
        else
        {
            if (!checkHttpDirective(tokens, i))
                return false;
        }
    }
    return true;
}
bool parser(std::string filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
        return false;
    std::stringstream ss;
    ss << file.rdbuf();
    file.close();
    std::vector<std::string> tokens;
    tokanize(ss, tokens);
    if (!countBrackets(tokens))
        return false;
    if (!validateBlocks(tokens))
        return false;
    return true;
}
int main() {
    
    try
    {
        if (!parser("config.conf"))
            throw std::runtime_error("Error parsing config file");
        // Http http("config.conf");
        // std::vector<ServerConfig> servers = http.getServers();
        // for (size_t i = 0; i < servers.size(); i++)
        // {
        //     std::cout << "Host: " << servers[i].host << std::endl;
        //     std::cout << "Port: " << servers[i].port << std::endl;
        //     std::cout << "Server Name: " << servers[i].serverName << std::endl;
        //     std::cout << "Error Pages: " << std::endl;
        //     for (auto it = servers[i].errorPages.begin(); it != servers[i].errorPages.end(); it++)
        //     {
        //         std::cout << it->first << " " << it->second << std::endl;
        //     }
        //     std::cout << "Client Body Size Limit: " << servers[i].clientBodySizeLimit << std::endl;
        //     std::cout << "Routes: " << std::endl;
        //     for (auto it = servers[i].routes.begin(); it != servers[i].routes.end(); it++)
        //     {
        //         std::cout << "Path: " << it->first << std::endl;
        //         std::cout << "Methods: ";
        //         for (size_t j = 0; j < it->second.methods.size(); j++)
        //         {
        //             std::cout << it->second.methods[j] << " ";
        //         }
        //         std::cout << std::endl;
        //         std::cout << "Redirect: " << it->second.redirect << std::endl;
        //         std::cout << "Root: " << it->second.root << std::endl;
        //         std::cout << "Directory Listing: " << it->second.directoryListing << std::endl;
        //         std::cout << "Default File: " << it->second.defaultFile << std::endl;
        //         std::cout << "CGI Path: " << it->second.cgiPath << std::endl;
        //         std::cout << "Upload Dir: " << it->second.uploadDir << std::endl;
        //     }
        // }   
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    return 0;
}
