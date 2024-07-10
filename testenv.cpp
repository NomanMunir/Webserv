#include <iostream>
#include <cstdlib>  // For getenv function

int main() {
    // Get the QUERY_STRING environment variable
    char *query_string = std::getenv("USER");
    if (query_string != nullptr) {
        std::cout << "Content-type: text/plain" << std::endl;
        std::cout << std::endl;
        std::cout << "QUERY_STRING: " << query_string << std::endl;
    } else {
        std::cout << "Content-type: text/plain" << std::endl;
        std::cout << std::endl;
        std::cout << "QUERY_STRING environment variable not found." << std::endl;
    }

    return 0;
}
