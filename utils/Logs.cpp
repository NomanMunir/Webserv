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
        std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();
        std::time_t time = std::chrono::system_clock::to_time_t(timestamp);
        std::string timeStr = std::ctime(&time);
        timeStr.pop_back();
        file << "[" << timeStr << "] " << "[" << type << "] " << message << std::endl;
    }
}
void Logs::close()
{
    if (file.is_open())
        file.close();
}
