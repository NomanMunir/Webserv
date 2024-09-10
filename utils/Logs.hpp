#ifndef LOGS_HPP
#define LOGS_HPP

#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <ctime>
#include "utils.hpp"

class Logs
{
	private:
		static std::ofstream file;
		Logs();

	public:
		static void init();
		static void appendLog(const std::string& type, const std::string& message);
		static void close();
};

#endif // LOGS_HPP
