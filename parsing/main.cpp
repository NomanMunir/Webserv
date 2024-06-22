#include <iostream>
#include <fstream>
#include <sstream>
#include "Config.hpp"
#include "ConfigParser.hpp"

ConfigValue::ConfigValue() : type(CONFIG_NULL) {}

void traverseConfig(const ConfigValue &value, int depth = 0) {
    switch (value.type) {
        case CONFIG_OBJECT:
            std::cout << std::string(depth, ' ') << "{\n";
            for (std::map<std::string, ConfigValue>::const_iterator it = value.objectValue.begin(); it != value.objectValue.end(); ++it) {
                std::cout << std::string(depth + 2, ' ') << "\"" << it->first << "\": ";
                traverseConfig(it->second, depth + 2);
            }
            std::cout << std::string(depth, ' ') << "}\n";
            break;
        case CONFIG_STRING:
            std::cout << std::string(depth, ' ') << "\"" << value.stringValue << "\"\n";
            break;
        case CONFIG_NUMBER:
            std::cout << std::string(depth, ' ') << value.numberValue << "\n";
            break;
        case CONFIG_BOOL:
            std::cout << std::string(depth, ' ') << (value.boolValue ? "true" : "false") << "\n";
            break;
        case CONFIG_NULL:
            std::cout << std::string(depth, ' ') << "null\n";
            break;
        default:
            break;
    }
}

void printConfig(const Config &config) {
    for (std::map<std::string, ConfigValue>::const_iterator it = config.root.begin(); it != config.root.end(); ++it) {
        std::cout << "\"" << it->first << "\": ";
        traverseConfig(it->second, 2);
    }
}

std::string readFile(const std::string &filename) {
    std::ifstream file(filename.c_str());
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file");
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

int main() {
    try {
        std::string filename = "config"; // Replace with your config file path
        std::string content = readFile(filename);
        ConfigParser parser(content);
        Config config = parser.parse();
        printConfig(config);
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << "\n";
    }

    return 0;
}
