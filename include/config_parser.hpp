#ifndef CONFIG_PARSER_HPP
#define CONFIG_PARSER_HPP

#include <string>
#include <map>

class Config {
public:
    std::map<std::string, std::string> global_settings;

    void load(const std::string& filePath); // Declare the load method
};

#endif // CONFIG_PARSER_HPP
