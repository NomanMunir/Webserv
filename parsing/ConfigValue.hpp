#ifndef CONFIGVALUE_HPP
#define CONFIGVALUE_HPP

#include <string>
#include <map>
#include <vector>

enum ConfigValueType { CONFIG_OBJECT, CONFIG_ARRAY, CONFIG_STRING, CONFIG_NUMBER, CONFIG_BOOL, CONFIG_NULL };

class ConfigValue {
public:
    ConfigValueType type;
    std::map<std::string, ConfigValue> objectValue;
    std::vector<ConfigValue> arrayValue;
    std::string stringValue;
    double numberValue;
    bool boolValue;

    ConfigValue();
};

#endif // CONFIGVALUE_HPP
