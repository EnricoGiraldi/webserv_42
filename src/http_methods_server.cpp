#include "server.hpp"
#include <iostream>

void Server::handlePostRequest(int clientSocket, const std::string& body) {
    std::cout << "Handling POST request with body: " << body << std::endl;
    sendResponse(clientSocket, 200, "text/plain", "POST request received: " + body);
}

void Server::handleDeleteRequest(int clientSocket, const std::string& path) {
    std::cout << "Handling DELETE request for path: " << path << std::endl;
    sendResponse(clientSocket, 200, "text/plain", "Resource deleted: " + path);
}
