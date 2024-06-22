#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include "Config.hpp"
#include <string>

class ConfigParser {
public:
    ConfigParser(const std::string &input);
    Config parse();

private:
    std::string input;
    size_t pos;

    void skipWhitespace();
    ConfigValue parseObject();
    ConfigValue parseValue();
    ConfigValue parseStringValue();
    ConfigValue parseNumberValue();
    ConfigValue parseBoolValue();
    ConfigValue parseNullValue();
    std::string parseString();
};

#endif // CONFIGPARSER_HPP
