#ifndef CGI_HPP
#define CGI_HPP

#include <map>
#include <string>
#include "../response/Response.hpp"


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
    int 												pid;
	int													pipeFd[2];
    std::string											_fullPath;
    Response &											_response;
    Request &                                           _request;
	// int													postBodyFd;
	// int													cgiClientSocket;
	// std::string											cgiResponseMessage;
	// std::chrono::time_point<std::chrono::steady_clock>	startTime;

	// bool												isValid;
    // Helper methods

    bool checkFilePermission(const char* path);
    void freeEnv(char** envp);
    void setCGIEnv();
    void vecToChar(std::vector<std::string> &envMaker);


    // Configuration map
    std::map<std::string, std::string> config_;
    char** _envp;
};

#endif // CGI_HANDLER_H
