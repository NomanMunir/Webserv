#ifndef LOGS_HPP
#define LOGS_HPP

#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include "utils.hpp"
#include <iomanip>
#include <sstream>

#define LOG_FILE "logs/logs.log"
class Logs
{
	private:
		Logs();
		static std::ofstream file;

	public:
		static void init();
		static void appendLog(const std::string& type, const std::string& message);
		static void close();
};

#endif // LOGS_HPP
