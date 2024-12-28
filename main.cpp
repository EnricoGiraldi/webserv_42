#include "server.hpp"
#include "config_parser.hpp"
#include <iostream>

int main() {
    try {
        // Load configuration
        Config config;
        config.global_settings["port"] = "8080";

        // Start the server
        Server server(config);
        server.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
