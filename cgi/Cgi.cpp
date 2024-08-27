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


Cgi::Cgi(Request &request, std::string fullPath, Response &response) 
    : _request(request),
    _fullPath(fullPath),
    _response(response) { }

Cgi::~Cgi()
{
    // Free allocated environment variables
    // freeEnv(_envp);
    // if (fd_out[0] != -1)
	// {
	// 	close(fd_out[0]);
	// 	fd_out[0] = -1;
	// }
	// if (fd_in[0] != -1)
	// {
	// 	close(fd_in[0]);
	// 	fd_in[0] = -1;
	// }

}

bool Cgi::checkFilePermission(const char* path) 
{
    struct stat fileStat;
    if (stat(path, &fileStat) < 0) 
    {
        std::cerr << "Error retrieving file stats: " << strerror(errno) << std::endl;
        return false;
    }
    return (fileStat.st_mode & S_IXUSR) || // Owner has execute permission
           (fileStat.st_mode & S_IXGRP) || // Group has execute permission
           (fileStat.st_mode & S_IXOTH);   // Others have execute permission
}

void Cgi::execute()
{
    // Check file permissions
    if (!checkFilePermission(this->_fullPath.c_str()))
        this->_response.setErrorCode(500, "Internal Server Error: CGI script is not executable");

    // int fd_in[2], fd_out[2];  // read fd[0]  write fd[1]
    if (pipe(fd_in) < 0 || pipe(fd_out) < 0)
        this->_response.setErrorCode(500, "Internal Server Error: Failed to create pipe");
    createEnv();
    pid = fork();

    // need to clean env and close fds if any error happens.
    if (pid < 0)
        this->_response.setErrorCode(500, "Internal Server Error: Failed to fork process");
    // check for the content length to know the data and handel error
    if (pid == 0)
    {
        std::string method;
        method =  _request.getHeaders().getValue("method");
        // Child process
        close(fd_in[1]);
        close(fd_out[0]); 
        if( method == "POST")
        {
            dup2(fd_in[0], STDIN_FILENO);
            close(fd_in[0]);
        }
        dup2(fd_out[1], STDOUT_FILENO);
        close(fd_out[1]);

        char buf[1024];
        ssize_t count = read(STDIN_FILENO ,buf, sizeof(buf) - 1); // change to read the size of Content-Length
        // std::cout << "count: " << count << std::endl;
        if(count > 0)
        {
            buf[count] = '\0';
            std::cerr << "data resived" << std::endl; 
        }
        else
            std::cerr << "data not resived" << std::endl;
        
        char* argv[] = { const_cast<char*>(this->_fullPath.c_str()), nullptr };

        if (execve(this->_fullPath.c_str(), argv, _envp) < 0)
        {
            std::cerr << "Failed to execute " << this->_fullPath << std::endl;
            freeEnv(_envp);
            exit(1);
        }
    } 
    else
    {
        // Parent process
        close(fd_in[0]);
        close(fd_out[1]);
        
        
        if(_request.getHeaders().getValue("method") == "POST" )
        {
            std::string num= "numan ali numan ali";
            std::string readBody =  _request.getHeaders().getValue("body");
            size_t nr = write(fd_in[1],&num, num.size());
            if (nr < 0)
            {
                std::cerr << "Error: " << std::endl;
            }
        }
        // if(errno == -1)  todo

        close(fd_in[1]);

        // std::string output;
        char buffer[1024];
        ssize_t count;

        while ((count = read(fd_out[0], buffer, sizeof(buffer))) > 0) 
        {
            // std::cout << count << std::endl;
            output.append(buffer, count);
        }
        close(fd_out[0]);

        int status;
        waitpid(pid, &status, 0);

    }
}

void Cgi::freeEnv(char** envp) 
{
    for (size_t i = 0; envp[i] != nullptr; ++i) 
        delete[] envp[i];
    delete[] envp;
}

void Cgi::vecToChar(std::vector<std::string> &envMaker)
{
    _envp = new char*[envMaker.size() + 1];
    if (!_envp)
        this->_response.setErrorCode(500, "Internal Server Error: Failed to allocate memory for environment variables");
    for (size_t i = 0; i < envMaker.size(); ++i) 
    {
        _envp[i] = new char[envMaker[i].length() + 1];
        if (!_envp[i])
            this->_response.setErrorCode(500, "Internal Server Error: Failed to allocate memory for environment variables");
        strcpy(_envp[i], envMaker[i].c_str());
    }
    _envp[envMaker.size()] = NULL;
}

void Cgi::createEnv()
{
    std::vector<std::string> envMaker;
    std::vector<std::string>::iterator it;

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
    envMaker.push_back("SERVER_PROTOCOL=" + _request.getHeaders().getValue("version"));
    envMaker.push_back("SERVER_PORT=" + _request.getHeaders().getValue("port"));
    envMaker.push_back("HTTP_HOST=" + _request.getHeaders().getValue("host"));

    // Print the environment variables for debugging
    // for (it = envMaker.begin(); it != envMaker.end(); ++it)
    //     std::cout << *it << std::endl;
    vecToChar(envMaker);
}


// int main() {
//     std::string method = "GET";
//     std::string query = "name=John&age=30";
//     std::string contentLength = "0";
//     std::string this->_fullPath = "e.cgi";
//     std::map<std::string, std::string > allConf;
//     allConf["REQUEST_METHOD"] = "GET";
//     allConf["QUERY_STRING"] = "name=John&age=30";
//     allConf["CONTENT_LENGTH"] = "15";
//     allConf["scriptPath"] = "testp.py";
//     allConf["postData"] = "Hi How are you today are you okay, are you taking exam today";

// // for(int i = 0; i < 10000; i++)
// // {
//      Cgi cgi(allConf);
//     cgi.execute();
// // }
   

//     return 0;
// }
