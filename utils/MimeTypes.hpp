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
    static std::map<std::string, std::string> mimeTypes; // Extension -> MIME type

    MimeTypes() {} // Private constructor to prevent instantiation
    static void parseMimeFile(const std::string &filename);
    static std::string trim(const std::string &str); // Helper to trim spaces
};

#endif // MIMETYPES_HPP
