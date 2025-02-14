#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include "config_parser.hpp"
#include "router.hpp"

class Server {
public:
    Server(const ConfigParser& configParser);
    ~Server();

    void run();
    void stop();
    void handleClient(int clientSocket);

    void handlePostRequest(int clientSocket, const std::string& body);
    void handleDeleteRequest(int clientSocket, const std::string& path);
    void handleCGIRequest(int clientSocket, const std::string& path, const std::string& query);

private:
    int _serverSocket;
    bool _isRunning;
    std::string _documentRoot;
    int _port;

    void setupSocket(int port);
    void serveStaticFile(int clientSocket, const std::string& path);
    std::string detectMimeType(const std::string& path);
    void sendResponse(int clientSocket, int statusCode, const std::string& contentType, const std::string& content);
    void sendErrorResponse(int clientSocket, int statusCode, const std::string& message);
    std::pair<std::string, std::string> parseRequest(const std::string& request);
};

std::string exampleHandler();

#endif // SERVER_HPP
