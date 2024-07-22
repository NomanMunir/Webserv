#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <map>
#include <sys/stat.h>

bool checkFilePermission(const char* path)
{
    struct stat fileStat;
    if (stat(path, &fileStat) < 0) 
    {
        std::cerr << "Error retrieving file stats: " << strerror(errno) << std::endl;
        return false;
    }
     bool canExecute = (fileStat.st_mode & S_IXUSR) || // Owner has execute permission
                       (fileStat.st_mode & S_IXGRP) || // Group has execute permission
                       (fileStat.st_mode & S_IXOTH); 
    return canExecute;
}

void freeEnv(char** envp) 
{
    for (size_t i = 0; envp[i] != nullptr; ++i) 
    {
        delete[] envp[i];
    }
    delete[] envp;
}

char** createEnv(std::map<std::string, std::string> allConf)
{
    std::map<std::string, std::string>::iterator it;
    
    size_t arraySize = allConf.size();
    char** envp;
    
    envp = new char*[arraySize + 1];
    size_t i = 0;
    for (it = allConf.begin(); it != allConf.end(); ++it)
    {
        std::string envEntry = it->first + "=" + it->second;
        // Allocate memory for the C-string and copy the content
        envp[i] = new char[envEntry.size() + 1];
        std::strcpy(envp[i], envEntry.c_str());  
        ++i;
    }
       
    envp[arraySize] = nullptr;
    
    return envp;

}

void    executeCgi(std::map<std::string, std::string> allConf)
{
    char** envp;
    std::string path;
    std::map<std::string, std::string>::iterator it = allConf.find("scriptPath");
    if (it != allConf.end())
        path = it->second;
    std::string query = allConf.at("QUERY_STRING");
    int fd[2];
    int pid = 0;
    if(checkFilePermission(path.c_str()) == false)
    {
        std::cerr << "you dont have permation to execute the file " << std::endl;
        return;
    }
        
    envp = createEnv(allConf);
    if(pipe(fd) < 0)
        std::cerr << "Error with pipe" << std::endl;
    pid = fork();
    if(pid < 0)
        std::cerr << "Error with fork" << std::endl;
    // if(allConf.at("REQUEST_METHOD") == "POST");
    if(pid == 0)
    {
        close(fd[0]);
        dup2(fd[1], 1);
        close(fd[1]);
        char* argv[] = { (char*)path.c_str(), nullptr };
        std::cout << path << std::endl;
        execve(path.c_str(), argv, envp);
        // execv(path.c_str(), argv);
        std::cerr << "Faild to execute" << std::endl;
    }
    else
    {
        close(fd[1]);
        ssize_t count = 0;
        char buffer[1024] = {0};
        std::string output;
        while ((count = read(fd[0], buffer, sizeof(buffer))) > 0)
        {
            output.append(buffer, count);
        }
        // std::cout << output << std::endl;
          close(fd[0]);
        
        int status;
        waitpid(pid, &status, 0);
        std::cout << output << std::endl;
    //      for (size_t i = 0; envp[i] != nullptr; ++i) 
    // {
    //     std::cout << envp[i] << std::endl;
    // }
    freeEnv(envp);
    }
}










int main()
{
   
    std::string method = "GET";
    std::string query = "name=John&age=30";
    std::string contentLength = "0";
    std::string scriptPath = "e.cgi";
    std::map<std::string, std::string > allConf;
    allConf["REQUEST_METHOD"] = "GET";
    allConf["QUERY_STRING"] = "name=John&age=30";
    allConf["CONTENT_LENGTH"] = "0";
    allConf["scriptPath"] = "try.py";
    // if (isCGIRequest(scriptPath)) {
    // setCGIEnvironment(method, query, contentLength);
    // settingEnvVariable(allMe);
    char** pase = createEnv(allConf);

    // for (size_t i = 0; pase[i] != nullptr; ++i) 
    // {
    //     std::cout << pase[i] << std::endl;
    // }
    // std::cout << 
    
    executeCgi(allConf);
}

