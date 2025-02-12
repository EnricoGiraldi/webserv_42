#include "config_parser.hpp"
#include "server.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: ./webserv <config_file.conf>" << std::endl;
        return 1;
    }

    ConfigParser configParser;
    if (!configParser.parseConfigFile(argv[1])) {
        std::cerr << "Failed to parse configuration file: " << argv[1] << std::endl;
        return 1;
    }

    Server server(configParser);
    server.run();

    return 0;
}
