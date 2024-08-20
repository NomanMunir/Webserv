#ifndef CGI_HPP
#define CGI_HPP

#include <map>
#include <string>
#include "../response/Response.hpp"


class Request;

class Cgi {
public:
    // Constructor
    Cgi(Request &request, char** env);
    std::string output;
    char** env;
    Request _request;
    int fd_in[2];
    int fd_out[2];
    // Destructor
    ~Cgi();

    // Method to execute the CGI script
    void execute();

private:
    int 												pid;
	int													pipeFd[2];
	// int													postBodyFd;
	// int													cgiClientSocket;
	// std::string											cgiResponseMessage;
	// std::chrono::time_point<std::chrono::steady_clock>	startTime;

	// bool												isValid;
    // Helper methods
   
    bool checkFilePermission(const char* path);
    void freeEnv(char** envp);
    char** createEnv();

    // Configuration map
    std::map<std::string, std::string> config_;
    char** _envp; // Environment variables for execve
};

#endif // CGI_HANDLER_H
