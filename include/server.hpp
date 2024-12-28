#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <map>
#include "config_parser.hpp"

class Server {
public:
    Server(const Config& config);
    ~Server();

    void run();
    void stop();

private:
    int _serverSocket;
    bool _isRunning;

    void setupSocket(const Config& config);
    void handleClient(int clientSocket);

    // Helper functions
    std::string parseRequestPath(const std::string& request);
    std::pair<std::string, std::string> parseRequest(const std::string& request);
    void logRequest(const char* buffer);
    void serveStaticFile(int clientSocket, const std::string& path);
    std::string detectMimeType(const std::string& path);
    void handleCGIRequest(int clientSocket, const std::string& path);
    void sendResponse(int clientSocket, int statusCode, const std::string& contentType, const std::string& content);
    void sendErrorResponse(int clientSocket, int statusCode, const std::string& message);

    // HTTP method handlers
    void handlePostRequest(int clientSocket, const std::string& body);
    void handleDeleteRequest(int clientSocket, const std::string& path);

    // Status message helper
    std::string getStatusMessage(int statusCode);
};

// Example handler declaration
std::string exampleHandler();

#endif // SERVER_HPP
