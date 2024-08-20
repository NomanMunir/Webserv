#include "Cookie.hpp"


Cookie::Cookie()
{
    initializeRandomSeed();
    //ctor
}

Cookie::~Cookie()
{
    //dtor
}

// check for if its has cookies or not and check for if it has you will raded ok 
// bool Cookie::hasCookie(Request &request )  // i need to pass request.hedares 
// {

// }

// thinking about add function that srand and check for if it create new srand ok
bool Cookie::checkForSession(const std::string& requestedSessionId)
{
    if(std::find(sessionList.begin(), sessionList.end(), requestedSessionId) != sessionList.end())
        return true;
    return false;
}




void Cookie::initializeRandomSeed() {
    static bool initialized = false;
    if (!initialized) {
        std::srand((std::time(0)));
        initialized = true;
    }
}


std::string Cookie::generateId(size_t length)
{
    // Define the characters that will be used in the session ID
    const std::string characters = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::string sessionId;

    // std::srand(static_cast<unsigned int>(std::time(0)));
    // std::cout << "max rand: " << RAND_MAX << std::endl;
    for(int i = 0; i < length; i++)
    {
        // std::cout << std::rand() << std::endl;
        int randomIndex = std::rand() % characters.size();
        // std::cout << " index: " << randomIndex << std::endl; 
        sessionId += characters[randomIndex];
    }
    return sessionId;
}


// need to create random session id and expired time ;
std::string Cookie::createSession()
{
    std::string sessionId;
    
    do{
        sessionId = generateId();
        // std::cout << sessionId << std::endl; 
    }while(std::find(sessionList.begin(), sessionList.end(), sessionId) != sessionList.end());

    sessionList.push_back(sessionId);
    // sessionList.push_back("ykb3G0AooWkF9RaggC29PAMF5CfJvW8t");
    return "sessionId=" + sessionId;

}

std::string Cookie::createCookie()
{
    std::stringstream finalCookie;
  
    finalCookie  << createSession();

    // for(std::string a : sessionList)
    //     std::cout << "id: " << a << std::endl;
        
    // finalCookie << "max-age=3600"; // 
    return finalCookie.str();

}


int main() {
    Cookie cookie;
    // for (int i = 0; i < 100; ++i) {
        std::cout << cookie.createSession() << std::endl;
    // }

    // if(cookie.checkForSession("ykb3G0AooWkF9RaggC29PAMF5CfJvW8tf"))
    //     std::cout << "success" << std::endl;
    // else
    //      std::cout << "False" << std::endl;

    return 0;
}