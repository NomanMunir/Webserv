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

Cgi::~Cgi()
{
    if (fd_out[0] != -1) close(fd_out[0]);
    if (fd_out[1] != -1) close(fd_out[1]);
    if (fd_in[0] != -1) close(fd_in[0]);
    if (fd_in[1] != -1) close(fd_in[1]);
}

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

void Cgi::checkCGITimeout(pid_t pid, Request &_request, Response &_response)
{
    int     status;
    int     childStatus = 0;
    double  time = 0;
    clock_t start = clock();
    while (childStatus == 0)
    {
        childStatus = waitpid(pid, &status, WNOHANG);
        clock_t end = clock();
        time = static_cast<double>(end - start) / CLOCKS_PER_SEC;
        
        if (time > CGI_TIMEOUT)
        {
            if (kill(pid, SIGKILL) < 0)
            {
                Logs::appendLog("ERROR", "[checkCGITimeout]\t\t Failed to kill CGI process " + std::string(strerror(errno)));
                freeEnv(_envp);
               _response.setErrorCode(500, "[checkCGITimeout]\t\t Internal Server Error: Failed to kill CGI process " + std::string(strerror(errno)));
            }
            Logs::appendLog("INFO", "[checkCGITimeout]\t\t CGI script execution timed out and was killed");
            freeEnv(_envp);
           _response.setErrorCode(504, "[checkCGITimeout]\t\t CGI script execution timed out");
        }
    }
}



void Cgi::execute(EventPoller *poller, Request &_request, Response &_response, std::string fullPath)
{
    this->_fullPath = fullPath;
    // Check file permissions
    if (!checkFilePermission(this->_fullPath.c_str()))
        _response.setErrorCode(500, "[execute]\t\t CGI script is not executable");

    if (pipe(fd_in) < 0)
        _response.setErrorCode(500, "[execute]\t\t Failed to create pipe");
    if (pipe(fd_out) < 0)
        _response.setErrorCode(500, "[execute]\t\t Failed to create pipe");
    setCGIEnv(_request, _response);
    this->_pid = fork();

    if (_pid < 0)
     {
        // Fork failed
        close(fd_in[0]); close(fd_in[1]);
        close(fd_out[0]); close(fd_out[1]);
        _response.setErrorCode(500, "[execute]\t\t Failed to fork process");
    }

    if (_pid == 0) 
    {
        // Child process
        close(fd_in[1]); // Close the write end of fd_in
        close(fd_out[0]); // Close the read end of fd_out

        dup2(fd_in[0], STDIN_FILENO);
        close(fd_in[0]);
        dup2(fd_out[1], STDOUT_FILENO);
        close(fd_out[1]);


        char* argv[] = { const_cast<char*>(_fullPath.c_str()), NULL };

        if (execve(_fullPath.c_str(), argv, _envp) < 0)
        {
            Logs::appendLog("ERROR", "[execute]\t\t Failed to execute " + std::string(strerror(errno)));
            freeEnv(_envp);
            exit(1);
        }
    } 
    else 
    {
        // Parent process
        close(fd_in[0]);
        close(fd_out[1]);
        setNoneBlocking(fd_in[1]);
        if (_request.getHeaders().getValue("method") == "POST") 
        {
            std::string body = _request.getBody().getContent();
            if (write(fd_in[1], body.c_str(), body.size()) < 0)
            {
                close(fd_in[1]); close(fd_out[0]); freeEnv(_envp);
                _response.setErrorCode(500, "[execute]\t\t Failed to write to pipe");
            }
        }
        close(fd_in[1]);

        poller->addToQueue(fd_out[0], READ_EVENT);
        // int status;
        // waitpid(pid, &status, WNOHANG);
        // checkCGITimeout(pid, _request, _response);

        // char buffer[1024];
        // ssize_t count;

        // while ((count = read(fd_out[0], buffer, sizeof(buffer))) > 0)
        //     output.append(buffer, count);
        // std::cout << "Output from CGI: " << output << std::endl;
        // close(fd_out[0]);
    }
}


void Cgi::freeEnv(char** envp) 
{
    for (size_t i = 0; envp[i] != nullptr; ++i) 
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

    // SERVER_PORT
    // SCRIPT_NAME
    // REMOTE_USER
    // REMOTE_HOST
    // PATH_INFO

    envMaker.push_back("REQUEST_URI=" + _request.getHeaders().getValue("uri"));
    envMaker.push_back("PATH_INFO=" + _fullPath);
    envMaker.push_back("REQUEST_METHOD=" + _request.getHeaders().getValue("method"));
    if(!queryString.empty())
        envMaker.push_back("QUERY_STRING=" + queryString);
    if(!contentLength.empty() && method == "POST") // check if the body is empty
    {
        if (_request.getHeaders().getValue("Content-Length").empty())
            envMaker.push_back("CONTENT_LENGTH=" + std::to_string(_request.getBody().getContent().length()));
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

Cgi::Cgi()
{
    fd_out[0] = -1; fd_out[1] = -1;
    fd_in[0] = -1; fd_in[1] = -1;
}

Cgi::Cgi(const Cgi &c) 
{
    *this = c;
}

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
    }
    return *this;
}

int Cgi::getReadFd() const
{
    return fd_out[0];
}

int Cgi::getPid() const
{
    return _pid;
}
