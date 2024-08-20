#ifndef COOKIE_HPP
#define COOKIE_HPP

#include <iostream>
#include <vector>
#include <ctime>
#include <sstream>
#include <algorithm>

class Request;

class Cookie {
    private:
        std::vector<std::string> sessionList;
        std::string sessionId;
    public:
        Cookie();
        ~Cookie();
        std::string createCookie();
        std::string createSession();
        std::string generateId(size_t length = 32);
        void initializeRandomSeed();
        // bool Cookie::hasCookie(std::string); // i need to pass request.hedares 
        bool checkForSession(const std::string& requestedSessionId);
        



};




#endif