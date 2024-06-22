#include "ConfigParser.hpp"
#include <stdexcept>
#include <cctype>
#include <cstdlib>

ConfigParser::ConfigParser(const std::string &input) : input(input), pos(0) {}

Config ConfigParser::parse() {
    Config config;
    while (pos < input.size()) {
        skipWhitespace();
        if (input.substr(pos, 6) == "server") {
            pos += 6;
            skipWhitespace();
            config.root["server"] = parseObject();
        } else {
            std::string key = parseString();
            skipWhitespace();
            pos++;  // skip '{'
            skipWhitespace();
            config.root[key] = parseValue();
            skipWhitespace();
        }
    }
    return config;
}

void ConfigParser::skipWhitespace() {
    while (pos < input.size() && isspace(input[pos])) {
        pos++;
    }
}

ConfigValue ConfigParser::parseObject() {
    ConfigValue value;
    value.type = CONFIG_OBJECT;
    skipWhitespace();
    while (input[pos] != '}') {
        skipWhitespace();
        std::string key = parseString();
        skipWhitespace();
        pos++;  // skip '{'
        skipWhitespace();
        value.objectValue[key] = parseValue();
        skipWhitespace();
        if (input[pos] == ';') {
            pos++;  // skip ';'
            skipWhitespace();
        }
    }
    pos++;  // skip '}'
    return value;
}

ConfigValue ConfigParser::parseValue() {
    skipWhitespace();
    ConfigValue value;
    if (input[pos] == '{') {
        value = parseObject();
    } else if (input[pos] == '"') {
        value = parseStringValue();
    } else if (isdigit(input[pos]) || input[pos] == '-') {
        value = parseNumberValue();
    } else if (input.substr(pos, 4) == "true" || input.substr(pos, 5) == "false") {
        value = parseBoolValue();
    } else if (input.substr(pos, 4) == "null") {
        value = parseNullValue();
    } else {
        value = parseStringValue();
    }
    return value;
}

ConfigValue ConfigParser::parseStringValue() {
    ConfigValue value;
    value.type = CONFIG_STRING;
    while (input[pos] != ';' && !isspace(input[pos])) {
        value.stringValue += input[pos++];
    }
    return value;
}

ConfigValue ConfigParser::parseNumberValue() {
    ConfigValue value;
    value.type = CONFIG_NUMBER;
    size_t start = pos;
    if (input[pos] == '-') pos++;
    while (isdigit(input[pos])) pos++;
    if (input[pos] == '.') pos++;
    while (isdigit(input[pos])) pos++;
    value.numberValue = std::strtod(input.substr(start, pos - start).c_str(), NULL);
    return value;
}

ConfigValue ConfigParser::parseBoolValue() {
    ConfigValue value;
    value.type = CONFIG_BOOL;
    if (input.substr(pos, 4) == "true") {
        value.boolValue = true;
        pos += 4;
    } else {
        value.boolValue = false;
        pos += 5;
    }
    return value;
}

ConfigValue ConfigParser::parseNullValue() {
    ConfigValue value;
    value.type = CONFIG_NULL;
    pos += 4;  // skip 'null'
    return value;
}

std::string ConfigParser::parseString() {
    std::string result;
    while (pos < input.size() && !isspace(input[pos]) && input[pos] != '{' && input[pos] != '}') {
        result += input[pos++];
    }
    return result;
}
