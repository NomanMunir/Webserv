#ifndef CGI_HPP
#define CGI_HPP

#include <map>
#include <string>
#include <signal.h>
#include <time.h>
#include "../response/Response.hpp"
#include "../utils/Logs.hpp"
#include "../events/EventPoller.hpp"

#define CGI_TIMEOUT 5

class Request;

class Cgi 
{
public:
    Cgi();
    Cgi(const Cgi &c);
    Cgi& operator=(const Cgi &c);
    ~Cgi();

    void execute(EventPoller *poller, Request &_request, Response &_response, std::string fullPath);
    int getReadFd() const;
    int getPid() const;
    bool getIsCGI() const;
    
    std::string output;

private:
    int             fd_in[2];
    int             fd_out[2];
    std::string     _fullPath;
    pid_t           _pid;
    bool            _isCGI;
    char**          _envp;

    bool checkFilePermission(const char* path);
    void freeEnv(char** envp);
    void setCGIEnv(Request &_request, Response &_response);
    void vecToChar(std::vector<std::string> &envMaker, Response &_response);

};

#endif // CGI_HANDLER_H
