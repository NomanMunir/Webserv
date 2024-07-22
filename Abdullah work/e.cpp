// #! /usr/bin/c++

#include <iostream>
#include <cstdlib>

int main() {
    std::cout << "Content-type: text/plain\n\n";
    std::cout << "REQUEST_METHOD: " << getenv("REQUEST_METHOD") << "\n";
    std::cout << "QUERY_STRING: " << getenv("QUERY_STRING") << "\n";
    // std::cout << "CONTENT_TYPE: " << getenv("CONTENT_TYPE") << "\n";
    std::cout << "CONTENT_LENGTH: " << getenv("CONTENT_LENGTH") << "\n";
    // std::cout << "SCRIPT_NAME: " << getenv("SCRIPT_NAME") << "\n";
    // std::cout << "SERVER_NAME: " << getenv("SERVER_NAME") << "\n";
    // std::cout << "SERVER_PORT: " << getenv("SERVER_PORT") << "\n";
    return 0;
}
