#include"Cgi.hpp"

Cgi::Cgi(){
    this->isValid = false;
    this->cgiClientSocket = -1;
    this->postBodyFd = -1;
    this->pid = -1;
    this->pipeFd[0] = -1;
    this->pipeFd[1] = -1;
}

Cgi::Cgi(std::string string)
{
    
}