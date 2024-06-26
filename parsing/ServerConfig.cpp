
#include "ServerConfig.hpp"

Http::~Http() {}
Http::Http() {}


std::string Http::trim(const std::string &s)
{
    size_t start = s.find_first_not_of(" \t\r\n");
    size_t end = s.find_last_not_of(" \t\r\n");
    return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
}


void Http::parseDirective(const std::string &line)
{
    size_t pos = line.find(' ');
    if (pos == std::string::npos)
        throw std::runtime_error("Invalid directive: " + line);
    std::string key = trim(line.substr(0, pos));
    std::string value = trim(line.substr(pos + 1));
    directives[key] = value;
}


std::vector<std::string> Http::split(const std::string &s, char delimiter)
{
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter))
    {
        tokens.push_back(trim(token));
    }
    return tokens;
}


void Http::parseRoute(std::ifstream &file, ServerConfig &serverConfig, const std::string &path)
{
    RouteConfig routeConfig;
    routeConfig.path = path;
    std::string line;
    while (std::getline(file, line))
    {
        line = trim(line);
        if (line.empty() || line[0] == '#')
            continue;
        if (line == "}")
            break;
        if (line.find("methods") == 0)
            routeConfig.methods = split(trim(line.substr(8)), ' ');
        else if (line.find("redirect") == 0)
            routeConfig.redirect = trim(line.substr(9));
        else if (line.find("root") == 0)
            routeConfig.root = trim(line.substr(5));
        else if (line.find("directory_listing") == 0)
            routeConfig.directoryListing = (trim(line.substr(18)) == "on");
        else if (line.find("default_file") == 0)
            routeConfig.defaultFile = trim(line.substr(13));
        else if (line.find("cgi") == 0)
            routeConfig.cgiPath = trim(line.substr(4));
        else if (line.find("upload_dir") == 0)
            routeConfig.uploadDir = trim(line.substr(11));
    }
    serverConfig.routes[path] = routeConfig;
}


void Http::parseServer(std::ifstream &file)
{
    ServerConfig serverConfig;
    std::string line;
    while (std::getline(file, line))
    {
        line = trim(line);
        if (line.empty() || line[0] == '#')
            continue;
        if (line == "}")
            break;
        if (line.find("host") == 0)
            serverConfig.host = trim(line.substr(5));
        else if (line.find("port") == 0)
            serverConfig.port = std::stoi(trim(line.substr(5)));
        else if (line.find("server_names") == 0)
            serverConfig.serverName = trim(line.substr(12));
        else if (line.find("error_page") == 0)
        {
            // error number can be more then 1; like: 500 501 502 /path/to/error
            // it can be like map<vector<int>, string> >;
        
            size_t pos = line.find(' ', 11);
            int code = std::stoi(trim(line.substr(11, pos - 11)));

            std::string path = trim(line.substr(pos + 1));
            serverConfig.errorPages[code] = path;

        }
        // need to store it as string not number. It can be 20M, 20K, 20G

        else if (line.find("client_body_size") == 0)
            serverConfig.clientBodySizeLimit = std::stoi(trim(line.substr(17)));
        else if (line.find("location") == 0)
        {
            size_t pos = line.find(' ');
            std::string path = trim(line.substr(pos + 1));
            parseRoute(file, serverConfig, path);
        }
        else
            parseDirective(line);
    }
    servers.push_back(serverConfig);
}

Http::Http(const std::string &filePath)
{
    std::ifstream file(filePath.c_str());
    if (!file.is_open())
        throw std::runtime_error("Unable to open config file: " + filePath);
    std::string line;
    while (std::getline(file, line))
    {
        line = trim(line);
        if (line.empty() || line[0] == '#')
            continue;
        if (line != "http {")
            throw std::runtime_error("Invalid config file format");
        break;
    }
    while (std::getline(file, line))
    {
        line = trim(line);
        if (line.empty() || line[0] == '#')
            continue;
        if (line == "}")
            break;
        if (line.find("server") == 0)
            parseServer(file);
        else
            parseDirective(line);
    }
}

const std::vector<ServerConfig>& Http::getServers() {
    return servers;
}

const std::map<std::string, std::string>& Http::getDirectives() {
    return directives;
}