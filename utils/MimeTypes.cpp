#include "MimeTypes.hpp"
#include <sstream>
#include <algorithm>

// Initialize static member
std::map<std::string, std::string> MimeTypes::mimeTypes;

void MimeTypes::load(const std::string &filename)
{
    parseMimeFile(filename);
}

void MimeTypes::parseMimeFile(const std::string &filename)
{
    std::ifstream file(filename.c_str());
    if (!file.is_open())
        throw std::runtime_error("Could not open mime.types file");

    std::string line;
    while (std::getline(file, line))
    {
        line = trim(line);

        if (line.empty() || line[0] == '#' || line == "types" || line == "{" || line == "}")
            continue;

        std::istringstream iss(line);
        std::string mimeType;
        std::string extension;

        if (iss >> mimeType)
        {
            while (iss >> extension)
            {
                if (extension.back() == ';')
                    extension.resize(extension.size() - 1);

                mimeTypes[extension] = mimeType;
            }
        }
    }
}

std::string MimeTypes::getType(const std::string &extension)
{
    std::map<std::string, std::string>::const_iterator it = mimeTypes.find(extension);
    if (it != mimeTypes.end())
    {
        return it->second;
    }
    return "application/octet-stream"; // Default MIME type if not found
}

std::string MimeTypes::trim(const std::string &str)
{
    const std::string whitespace = " \t\n\r";
    size_t start = str.find_first_not_of(whitespace);
    size_t end = str.find_last_not_of(whitespace);

    if (start == std::string::npos || end == std::string::npos)
    {
        return ""; // Empty or all whitespace string
    }
    return str.substr(start, end - start + 1);
}
