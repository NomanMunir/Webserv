#include "Cgi.hpp"
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <cstring>
#include <cerrno>
#include <iostream>
#include <vector>
#include "../response/Response.hpp"

bool Cgi::checkFilePermission(const char* path) 
{
    struct stat fileStat;
    if (stat(path, &fileStat) < 0) 
    {
        Logs::appendLog("ERROR", "[checkFilePermission]\t\t " + std::string(strerror(errno)));
        return false;
    }
    return (fileStat.st_mode & S_IXUSR) || // Owner has execute permission
           (fileStat.st_mode & S_IXGRP) || // Group has execute permission
           (fileStat.st_mode & S_IXOTH);   // Others have execute permission
}

bool setNoneBlocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
    {
        Logs::appendLog("ERROR", "[Cgi setNoneBlocking]\t\t " + std::string(strerror(errno)));
        return false;
    }
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
    {
        Logs::appendLog("ERROR", "[Cgi setNoneBlocking]\t\t " + std::string(strerror(errno)));
        return false;
    }
    return true;
}

void Cgi::execute(EventPoller *poller, Request &_request, Response &_response, std::string fullPath)
{
    this->_fullPath = fullPath;
    if (!checkFilePermission(this->_fullPath.c_str()))
        _response.setErrorCode(500, "[execute]\t\t CGI script is not executable");

    if (pipe(fd_in) < 0)
        _response.setErrorCode(500, "[execute]\t\t Failed to create pipe");
    if (pipe(fd_out) < 0)
        _response.setErrorCode(500, "[execute]\t\t Failed to create pipe");
    setCGIEnv(_request, _response);
    this->_isCGI = true;
    this->_pid = fork();

    if (_pid < 0)
     {
        close(fd_in[0]); close(fd_in[1]);
        close(fd_out[0]); close(fd_out[1]);
        _response.setErrorCode(500, "[execute]\t\t Failed to fork process");
    }

    if (_pid == 0) 
    {
        close(fd_in[1]);
        close(fd_out[0]);

        dup2(fd_in[0], STDIN_FILENO);
        close(fd_in[0]);
        dup2(fd_out[1], STDOUT_FILENO);
        close(fd_out[1]);


        char* argv[] = { const_cast<char*>(_fullPath.c_str()), NULL };
        Logs::appendLog("INFO", "[execute]\t\t Executing " + _fullPath + " with PID " + intToString(getpid()) + " and fd " + intToString(_response.getClientSocket()));
        if (execve(_fullPath.c_str(), argv, _envp) < 0)
        {
            Logs::appendLog("ERROR", "[execute]\t\t Failed to execute " + std::string(strerror(errno)));
            freeEnv(_envp);
            exit(1);
        }
    } 
    else 
    {
        close(fd_in[0]);
        close(fd_out[1]);
        if (!setNoneBlocking(fd_in[1]) || !setNoneBlocking(fd_out[0]))
        {
            close(fd_in[1]); close(fd_out[0]); freeEnv(_envp);
            _response.setErrorCode(500, "[execute]\t\t Failed to set non-blocking mode");
        }
        if (_request.getHeaders().getValue("method") == "POST") 
        {
            std::string body = _request.getBody().getContent();
            if (write(fd_in[1], body.c_str(), body.size()) < 0)
            {
                close(fd_in[1]); close(fd_out[0]); freeEnv(_envp);
                _response.setErrorCode(500, "[execute]\t\t Failed to write to pipe");
            }
            Logs::appendLog("INFO", "[execute]\t\t Writing to pipe " + intToString(fd_in[1]) + " with body size " + intToString(body.size()));
        }
        close(fd_in[1]);

        poller->addToQueue(fd_out[0], READ_EVENT);
        freeEnv(_envp);
    }
}


void Cgi::freeEnv(char** envp) 
{
    for (size_t i = 0; envp[i] != NULL; ++i) 
        if (envp[i]) delete[] envp[i];
    if (envp) delete[] envp;
}

void Cgi::vecToChar(std::vector<std::string> &envMaker, Response &_response)
{
    _envp = new char*[envMaker.size() + 1];
    if (!_envp)
        _response.setErrorCode(500, "[vecToChar]\t\t Failed to allocate memory for environment variables");
    for (size_t i = 0; i < envMaker.size(); ++i) 
    {
        _envp[i] = new char[envMaker[i].length() + 1];
        if (!_envp[i])
            _response.setErrorCode(500, "[vecToChar]\t\t Failed to allocate memory for environment variables");
        strcpy(_envp[i], envMaker[i].c_str());
    }
    _envp[envMaker.size()] = NULL;
}

void Cgi::setCGIEnv(Request &_request, Response &_response)
{
    std::vector<std::string>::iterator it;
    std::vector<std::string>& envMaker = _request.getSystemENV();

    std::string method =  _request.getHeaders().getValue("method");
    std::string contentLength = _request.getHeaders().getValue("Content-Length");
    std::string queryString = _request.getHeaders().getValue("query_string");

    envMaker.push_back("REQUEST_URI=" + _request.getHeaders().getValue("uri"));
    envMaker.push_back("PATH_INFO=" + _fullPath);
    envMaker.push_back("REQUEST_METHOD=" + _request.getHeaders().getValue("method"));
    if(!queryString.empty())
        envMaker.push_back("QUERY_STRING=" + queryString);
    if(!contentLength.empty() && method == "POST") // check if the body is empty
    {
        if (_request.getHeaders().getValue("Content-Length").empty())
            envMaker.push_back("CONTENT_LENGTH=" + intToString(_request.getBody().getContent().length()));
        else
            envMaker.push_back("CONTENT_LENGTH=" + _request.getHeaders().getValue("Content-Length"));
    }
    envMaker.push_back("HTTP_COOKIE=" +  _request.getHeaders().getValue("Cookie"));
    envMaker.push_back("SCRIPT_NAME=" +  _fullPath.substr(_fullPath.find_last_of("/") + 1));
    envMaker.push_back("CONTENT_TYPE=" +  _request.getHeaders().getValue("Content-Type"));
    envMaker.push_back("HTTP_USER_AGENT=" + _request.getHeaders().getValue("User-Agent"));
    envMaker.push_back("GATEWAY_INTERFACE=CGI/1.1");
    envMaker.push_back("SERVER_NAME=Nginx 3.0");
    envMaker.push_back("SERVER_PROTOCOL=" + _request.getHeaders().getValue("ersion"));
    envMaker.push_back("SERVER_PORT=" + _request.getHeaders().getValue("Port"));
    envMaker.push_back("HTTP_HOST=" + _request.getHeaders().getValue("Host"));

    vecToChar(envMaker, _response);
}

Cgi::Cgi(): _pid(-1), _isCGI(false), _envp(NULL)
{
    fd_out[0] = -1; fd_out[1] = -1;
    fd_in[0] = -1; fd_in[1] = -1;
}

Cgi::Cgi(const Cgi &c) { *this = c; }

Cgi& Cgi::operator=(const Cgi &c)
{
    if (this != &c) 
    {
        this->_fullPath = c._fullPath;
        this->output = c.output;
        this->_envp = c._envp;
        this->fd_in[0] = c.fd_in[0];
        this->fd_in[1] = c.fd_in[1];
        this->fd_out[0] = c.fd_out[0];
        this->fd_out[1] = c.fd_out[1];
        this->_pid = c._pid;
        this->_isCGI = c._isCGI;
    }
    return *this;
}

int Cgi::getReadFd() const { return fd_out[0]; }

int Cgi::getPid() const { return _pid; }

bool Cgi::getIsCGI() const { return _isCGI; }

Cgi::~Cgi()
{ }
