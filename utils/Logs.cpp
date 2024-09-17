#include "Logs.hpp"

std::ofstream Logs::file;

Logs::Logs() {}

void Logs::init()
{
    if (!file.is_open()) 
	{
        file.open("logs.log", std::ios::trunc);
        if (!file.is_open())
            std::cerr << "Error: Could not open log file" << std::endl;
    }
}

void Logs::appendLog(const std::string& type, const std::string& message)
{
    if (file.is_open()) 
    {
        std::time_t now = std::time(0); 
        char timeStr[100];
        std::strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", std::localtime(&now)); 

        std::string typeMode = "";
        std::string messageMode = "";
        std::string func = split(message, '\t')[0];   
        std::string messageStr = split(message, '\t')[2];

        if (type.size() < 5)
            typeMode = " ";
        if (func.size() < 30)
            messageMode.append(30 - func.size(), ' ');

        file << "[" << timeStr << "] " << "[" << type  << "] " << typeMode << func << messageMode << messageStr << std::endl;
    }
}

void Logs::close()
{
    if (file.is_open())
        file.close();
}
