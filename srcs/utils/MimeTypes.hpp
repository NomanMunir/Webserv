#ifndef MIMETYPES_HPP
#define MIMETYPES_HPP

#include <map>
#include <string>
#include <fstream>
#include <iostream>
#include <stdexcept>

class MimeTypes
{
public:
    static void load(const std::string &filename);
    static std::string getType(const std::string &extension);

private:
    MimeTypes() {}
    static std::map<std::string, std::string> mimeTypes;
    static void parseMimeFile(const std::string &filename);
    static std::string trim(const std::string &str);
};

#endif // MIMETYPES_HPP
