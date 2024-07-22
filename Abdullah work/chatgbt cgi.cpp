#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <sys/wait.h>
#include <sstream>

// bool isCGIRequest(const std::string& uri) {
//     return uri.find(".cgi") != std::string::npos;
// }

void setCGIEnvironment(const std::string& method, const std::string& query, const std::string& contentLength) {
    setenv("REQUEST_METHOD", method.c_str(), 1);
    setenv("QUERY_STRING", query.c_str(), 1);
    setenv("CONTENT_LENGTH", contentLength.c_str(), 1);
}

void executeCGIScriptWithPipes(const std::string& scriptPath) {
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        std::cerr << "Pipe failed" << std::endl;
        return;
    }
    
    pid_t pid = fork();
    
    if (pid < 0) {
        std::cerr << "Fork failed" << std::endl;
        return;
    }
    
    if (pid == 0) { // Child process
        close(pipefd[0]); // Close unused read end
        dup2(pipefd[1], STDOUT_FILENO); // Redirect stdout to the pipe
        close(pipefd[1]);
        
        char* argv[] = { (char*)scriptPath.c_str(), nullptr };
        execv(scriptPath.c_str(), argv);
        
        std::cerr << "Exec failed" << std::endl;
        exit(1);
    } else { // Parent process
        close(pipefd[1]); // Close unused write end
        char buffer[1024];
        ssize_t count;
        std::string output;
        while ((count = read(pipefd[0], buffer, sizeof(buffer))) > 0) {
            output.append(buffer, count);
        }
        close(pipefd[0]);
        
        int status;
        waitpid(pid, &status, 0);
        std::cout << output << std::endl;
        // handleCGIOutput(output);
    }
}

void handleCGIOutput(const std::string& output) {
    std::istringstream stream(output);
    std::string line;
    bool headersParsed = false;
    
    while (std::getline(stream, line)) {
        if (line == "\r" || line == "") {
            headersParsed = true;
            continue;
        }
        
        if (headersParsed) {
            std::cout << line << std::endl; // Content
        } else {
            // Parse headers (e.g., Content-Type)
            std::cout << "Header: " << line << std::endl;
        }
    }
}

int main() {
    std::string method = "GET";
    std::string query = "name=John&age=30";
    std::string contentLength = "0";
    std::string scriptPath = "try.py";
    
    // if (isCGIRequest(scriptPath)) {
        setCGIEnvironment(method, query, contentLength);
        executeCGIScriptWithPipes(scriptPath);
    // }
    
    return 0;
}
