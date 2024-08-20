#include <iostream>
#include <random>
#include <sstream>
#include <iomanip>
#include <string>
#include <algorithm>
#include <unordered_set>


std::string generateSessionId(size_t length = 32) {
    // Define the characters that will be used in the session ID
    const std::string characters = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::string sessionId;
    
    // Seed the random number generator
    std::srand(static_cast<unsigned int>(std::time(0)));
    
    // Generate the session ID
    for (size_t i = 0; i < length; ++i) {
        int index = std::rand() % characters.size();
        sessionId += characters[index];
    }
    
    return sessionId;
}


std::string createUniqueSessionId(std::unordered_set<std::string>& activeSessionIds) {
    std::string sessionId;
    do {
        sessionId = generateSessionId();  // Assume generateSessionId() generates a random session ID
    } while (activeSessionIds.find(sessionId) != activeSessionIds.end());

    // Once a unique ID is found, add it to the active session list
    activeSessionIds.insert(sessionId);
    return sessionId;
}




int main()
{
    
    std::string sessionId = generateSessionId();
    std::cout << "Generated Session ID: " << sessionId << std::endl;
    return 0;

       

    
}