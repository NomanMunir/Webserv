#ifndef UTILS_HPP
#define UTILS_HPP

#include "../parsing/Parser.hpp"
#include <string>
#include <vector>
#include <netinet/in.h>
#include <map>
#include <iostream>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cerrno>
#include <ctime>
#include <iomanip>
#include "Logs.hpp"
#include <fcntl.h>

#define IS_DIR 1
#define IS_FILE 2
#define IS_ERR 3

struct ServerConfig;

int isFileDir(const std::string &path);
std::string trim(const std::string &s);
std::string join(const std::vector<std::string> &tokens, char delimiter);
std::string trimChar(const std::string s, char c);
std::string getStatusMsg(std::string errorCode);
std::string getCurrentTimestamp();
std::string toUpperCase(const std::string &str);
std::string generateFullPath(std::string rootPath, std::string path, std::string routeName);
std::string intToString(int number);
std::vector<std::string> split(const std::string &s, char delimiter);
void isFile(const std::string &path);
void printServers(std::vector<ServerConfig> servers);
void removeCharsFromString(std::string &str, std::string charsToRemove);
void initializeVector(std::vector<std::string> &v, const std::string s[], size_t size);
void initializeVector(std::vector<std::string> &v, std::string s[], size_t size);
bool myFind(std::vector<std::string> &v, std::string &s);
bool validateNumber(std::string key, std::string value);

#endif // UTILS_HPP	