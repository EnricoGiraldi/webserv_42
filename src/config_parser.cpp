#include "config_parser.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>

void Config::load(const std::string& filePath) {
    std::ifstream file(filePath.c_str());
    if (!file.is_open()) {
        throw std::runtime_error("Unable to open configuration file: " + filePath);
    }

    std::string line;
    while (std::getline(file, line)) {
        // Remove comments
        size_t commentPos = line.find('#');
        if (commentPos != std::string::npos) {
            line = line.substr(0, commentPos);
        }

        // Trim whitespace
        size_t start = line.find_first_not_of(" \t");
        size_t end = line.find_last_not_of(" \t");
        if (start == std::string::npos) continue; // Skip empty lines
        line = line.substr(start, end - start + 1);

        // Parse key-value pairs
        size_t delimiterPos = line.find('=');
        if (delimiterPos == std::string::npos) continue; // Skip invalid lines
        std::string key = line.substr(0, delimiterPos);
        std::string value = line.substr(delimiterPos + 1);

        // Trim key and value
        start = key.find_first_not_of(" \t");
        end = key.find_last_not_of(" \t");
        key = key.substr(start, end - start + 1);

        start = value.find_first_not_of(" \t");
        end = value.find_last_not_of(" \t");
        value = value.substr(start, end - start + 1);

        global_settings[key] = value;
    }

    file.close();
}
