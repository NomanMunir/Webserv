#ifndef CGI_HPP
#define CGI_HPP


#include "ourHeader.hpp"




class Cgi{
    private:
    int 												pid;
	int													pipeFd[2];
	int													postBodyFd;
	int													cgiClientSocket;
	std::string											cgiResponseMessage;
	// std::chrono::time_point<std::chrono::steady_clock>	startTime;

	bool												isValid;  




    public:
    Cgi(); // it will take HttpRequest and ServerConfig ok
    Cgi(std::string string);
    ~Cgi();
    

};



#endif