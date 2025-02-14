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

    // Gestione CGI
    void handleCGIRequest(int clientSocket, const std::string& path, const std::string& query);

private:
    int _serverSocket;
    bool _isRunning;
    std::string _documentRoot;
    int _port;
    static const size_t MAX_UPLOAD_SIZE = 1024 * 1024; // 1 MB

    void setupSocket(int port);
    // Il parametro isHead indica se la richiesta è HEAD
    void serveStaticFile(int clientSocket, const std::string& path, bool isHead);
    std::string detectMimeType(const std::string& path);
    // sendResponse ora accetta un flag per inviare o meno il body
    void sendResponse(int clientSocket, int statusCode, const std::string& contentType,
                      const std::string& content, bool sendBody = true);
    void sendErrorResponse(int clientSocket, int statusCode, const std::string& message);
};

std::string exampleHandler();

#endif // SERVER_HPP
