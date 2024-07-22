#ifndef CGI_HANDLER_H
#define CGI_HANDLER_H

#include <map>
#include <string>

class Cgi {
public:
    // Constructor
    Cgi(const std::map<std::string, std::string>& config);

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
    char** createEnv( std::map<std::string, std::string>& conf);

    // Configuration map
    std::map<std::string, std::string> config_;
    char** envp_; // Environment variables for execve
};

#endif // CGI_HANDLER_H
