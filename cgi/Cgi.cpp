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


Cgi::Cgi(Request &request, char** env)
    : _request(request), env(env)
    {
    // Initialize environment variables
    // _envp = createEnv();
}

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

void Cgi::execute() 
{
    // i need to get the root of the script path
    std::string scriptPath = "/Users/absalem/Desktop/num/cgi-bin/test.py"; // todo  Validate the scriptPath 

    // Check file permissions
    if (!checkFilePermission("/Users/absalem/Desktop/num/cgi-bin/test.py"))
    {
        std::cerr << "You don't have permission to execute the file" << std::endl;
        return;
    }

    // int fd_in[2], fd_out[2];  // read fd[0]  write fd[1]
    if (pipe(fd_in) < 0 || pipe(fd_out) < 0)
    {
        std::cerr << "Error creating pipe" << std::endl;
        return;
    }
    _envp = createEnv();
    if(!_envp)
        std::cerr << "Error env empty" << std::endl;
    pid = fork();
    if (pid < 0) 
    {
        std::cerr << "Error with fork" << std::endl;
        return;
    }
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
        
        char* argv[] = { const_cast<char*>(scriptPath.c_str()), nullptr };

        if (execve(scriptPath.c_str(), argv, _envp) < 0)
        // {
        //     // freeEnv(env);
        //     exit(1);
        // }
        freeEnv(_envp);
        freeEnv(argv);
        std::cerr << "Failed to execute " << scriptPath << std::endl;
        exit(EXIT_FAILURE);
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

void Cgi::freeEnv(char** envp) 
{
    for (size_t i = 0; envp[i] != nullptr; ++i) 
        delete[] envp[i];
    delete[] envp;
}

char** Cgi::createEnv() 
{
    std::vector<std::string> envMaker;
    std::vector<std::string>::iterator it;

    std::string method =  _request.getHeaders().getValue("method");
    std::string contentLength = _request.getHeaders().getValue("Content-Length");
    std::string queryString = _request.getHeaders().getValue("query");
    // When the server executes a CGI script, 
    // it should set the PATH_INFO environment variable to the full path of the requested file.
    std::string pathInfo = _request.getHeaders().getValue("uri"); // i need to add the root

    // SERVER_PORT
    // SCRIPT_NAME
    // REMOTE_USER
    // REMOTE_HOST
    // PATH_INFO

    // Include More Environment Variables: Add more environment variables like SERVER_PROTOCOL,
    //  REMOTE_ADDR, REMOTE_PORT, and SERVER_PORT as they provide 

    envMaker.push_back("REQUEST_METHOD=" + _request.getHeaders().getValue("method"));
    // If the CGI script does not specify a Content-Length in its response, 
    // the server should treat EOF as the end of the returned data.
    if(!contentLength.empty() && method == "POST") // check if the body is empty
        envMaker.push_back("CONTENT_LENGTH=" + _request.getHeaders().getValue("Content-Length"));
    // envMaker.push_back("HTTP_COOKIE=" +  _request.getHeaders().getValue("Cookie"));
    if(!queryString.empty())
        envMaker.push_back("QUERY_STRING=" + _request.getHeaders().getValue("query"));
    envMaker.push_back("SCRIPT_NAME=" +  _request.getHeaders().getValue("uri"));
    envMaker.push_back("CONTENT_TYPE=" +  _request.getHeaders().getValue("Content-Type"));
    envMaker.push_back("HTTP_USER_AGENT=" + _request.getHeaders().getValue("User-Agent"));
    envMaker.push_back("GATEWAY_INTERFACE=CGI/1.1");
    envMaker.push_back("PATH_INFO=" + pathInfo);
    envMaker.push_back("SERVER_NAME=Nginx 3.0");
    envMaker.push_back("SERVER_PROTOCOL=" + _request.getHeaders().getValue("version"));
    // envMaker.push_back("SERVER_PORT=" + _request.getHeaders().getValue("port"));
    // envMaker.push_back("PATH_INFO=" + _request.getHeaders().getValue("path")); /// need to check later;
    envMaker.push_back("SCRIPT_FILENAME=/Users/absalem/Desktop/num" );
    // envMaker.push_back("HTTP_HOST=" + host);

    // Print the environment variables for debugging
    // for (const std::string& env : envMaker) {
    //     std::cout << env << std::endl;
    // // }

    // Convert std::vector<std::string> to char** 
    char** envp = new char*[envMaker.size() + 1];
    size_t i = 0;

    for (it = envMaker.begin(); it != envMaker.end(); ++it)
    {
        std::string envEntry = *it;
        // Allocate memory for the C-string and copy the content
        envp[i] = NULL;
        envp[i] = new char[envEntry.size() + 1];
        // bzero(envp[i], it->size() + 1);
        std::strcpy(envp[i], envEntry.c_str());  
        ++i;
    }

    envp[i] = NULL; // Null-terminate the array
    return envp; 
}


// int main() {
//     std::string method = "GET";
//     std::string query = "name=John&age=30";
//     std::string contentLength = "0";
//     std::string scriptPath = "e.cgi";
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
