#ifndef CGI_HPP
#define CGI_HPP

#include <map>
#include <string>
#include <signal.h>
#include <time.h>
#include "../response/Response.hpp"

#define CGI_TIMEOUT 5

class Request;

class Cgi 
{
public:

    Cgi(Request &request, std::string fullPath, Response &response);
    std::string output;
    int fd_in[2];
    int fd_out[2];

    ~Cgi();

    void execute();

private:
	int													pipeFd[2];
    std::string											_fullPath;
    Response &											_response;
    Request &                                           _request;

    bool checkFilePermission(const char* path);
    void freeEnv(char** envp);
    void setCGIEnv();
    void vecToChar(std::vector<std::string> &envMaker);
    void checkCGITimeout(pid_t pid);

    char** _envp;
};

#endif // CGI_HANDLER_H
