#include "Cgi.hpp"
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <cstring>
#include <cerrno>
#include <iostream>

Cgi::Cgi(const std::map<std::string, std::string>& config)
    : config_(config) 
    {
    // Initialize environment variables
    envp_ = createEnv(config_);
}

Cgi::~Cgi() 
{
    // Free allocated environment variables
    freeEnv(envp_);
}

void Cgi::execute() 
{
    std::string scriptPath = config_.at("scriptPath");

    // Check file permissions
    if (!checkFilePermission(scriptPath.c_str())) 
    {
        std::cerr << "You don't have permission to execute the file" << std::endl;
        return;
    }

    int fd[2];
    if (pipe(fd) < 0) 
    {
        std::cerr << "Error creating pipe" << std::endl;
        return;
    }

    pid = fork();
    if (pid < 0) 
    {
        std::cerr << "Error with fork" << std::endl;
        return;
    }

    if (pid == 0) 
    {
        // Child process
        close(fd[0]);
        dup2(fd[1], STDOUT_FILENO);
        close(fd[1]);

        char* argv[] = { const_cast<char*>(scriptPath.c_str()), nullptr };
        execve(scriptPath.c_str(), argv, envp_);
        std::cerr << "Failed to execute " << scriptPath << std::endl;
        exit(EXIT_FAILURE);
    } else 
    {
        // Parent process
        close(fd[1]);

        std::string output;
        char buffer[1024];
        ssize_t count;

        while ((count = read(fd[0], buffer, sizeof(buffer))) > 0) 
        {
            output.append(buffer, count);
        }
        close(fd[0]);

        int status;
        waitpid(pid, &status, 0);
        std::cout << output << std::endl;
    }
}

bool Cgi::checkFilePermission(const char* path) {
    struct stat fileStat;
    if (stat(path, &fileStat) < 0) {
        std::cerr << "Error retrieving file stats: " << strerror(errno) << std::endl;
        return false;
    }
    return (fileStat.st_mode & S_IXUSR) || // Owner has execute permission
           (fileStat.st_mode & S_IXGRP) || // Group has execute permission
           (fileStat.st_mode & S_IXOTH);   // Others have execute permission
}

void Cgi::freeEnv(char** envp) {
    for (size_t i = 0; envp[i] != nullptr; ++i) {
        delete[] envp[i];
    }
    delete[] envp;
}

char** Cgi::createEnv(std::map<std::string, std::string>& conf) 
{
    std::map<std::string, std::string>::iterator it;

    char** envp = new char*[conf.size() + 1];
    size_t i = 0;

    for (it = conf.begin(); it != conf.end(); ++it)
    {
        std::string envEntry = it->first + "=" + it->second;
        // Allocate memory for the C-string and copy the content
        envp[i] = new char[envEntry.size() + 1];
        std::strcpy(envp[i], envEntry.c_str());  
        ++i;
    }

    envp[i] = nullptr;
    return envp;
}


int main() {
    std::string method = "GET";
    std::string query = "name=John&age=30";
    std::string contentLength = "0";
    std::string scriptPath = "e.cgi";
    std::map<std::string, std::string > allConf;
    allConf["REQUEST_METHOD"] = "GET";
    allConf["QUERY_STRING"] = "name=John&age=30";
    allConf["CONTENT_LENGTH"] = "0";
    allConf["scriptPath"] = "try.py";

    Cgi cgi(allConf);
    cgi.execute();

    return 0;
}
