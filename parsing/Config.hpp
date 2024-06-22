#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "ConfigValue.hpp"
#include <map>
#include <string>

class Config {
public:
    std::map<std::string, ConfigValue> root;
};

#endif // CONFIG_H
