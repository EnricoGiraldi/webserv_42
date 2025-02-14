#include "config_parser.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cctype>

ConfigParser::ConfigParser() {
}

ConfigParser::~ConfigParser() {
}

std::string ConfigParser::trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    size_t end = s.find_last_not_of(" \t\r\n");
    return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
}

bool ConfigParser::parseConfigFile(const std::string& filePath) {
    std::ifstream configFile(filePath.c_str());
    if (!configFile.is_open()) {
        std::cerr << "Failed to open config file: " << filePath << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(configFile, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#')
            continue;
        if (line == "server {") {
            parseBlock(configFile, "server");
        }
    }
    return true;
}

void ConfigParser::parseBlock(std::istream& configStream, const std::string& /*blockName*/) {
    std::string line;
    std::string currentLocation;
    while (std::getline(configStream, line)) {
        line = trim(line);
        if (line == "}")
            break;
        if (line.empty() || line[0] == '#')
            continue;
        size_t pos = line.find("=");
        if (pos != std::string::npos) {
            std::string key = trim(line.substr(0, pos));
            std::string value = trim(line.substr(pos + 1));
            if (key == "location") {
                currentLocation = value;
                locations[currentLocation] = std::map<std::string, std::string>();
            } else if (!currentLocation.empty()) {
                locations[currentLocation][key] = value;
            } else {
                globalSettings[key] = value;
            }
        }
    }
}

const std::map<std::string, std::string>& ConfigParser::getGlobalSettings() const {
    return globalSettings;
}

const std::map<std::string, std::map<std::string, std::string> >& ConfigParser::getLocations() const {
    return locations;
}
