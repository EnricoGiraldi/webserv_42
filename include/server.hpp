#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <map>
#include "config_parser.hpp"
#include "router.hpp"

class Server {
public:
    Server(const ConfigParser& configParser);
    ~Server();

    void run();
    void stop();

    // Gestione delle richieste:
    void handlePostRequest(int clientSocket, const std::string& body);
    void handleDeleteRequest(int clientSocket, const std::string& path);
    void handleCGIRequest(int clientSocket, const std::string& path);  // Aggiunta la dichiarazione

private:
    int _serverSocket;
    bool _isRunning;

    void setupSocket(int port);
    void handleClient(int clientSocket);
    void serveStaticFile(int clientSocket, const std::string& path);
    std::string detectMimeType(const std::string& path);
    void sendResponse(int clientSocket, int statusCode, const std::string& contentType, const std::string& content);
    void sendErrorResponse(int clientSocket, int statusCode, const std::string& message);
    std::pair<std::string, std::string> parseRequest(const std::string& request);
    void logRequest(const char* buffer);
    std::string getStatusMessage(int statusCode);
};

// Funzione di esempio per il routing.
std::string exampleHandler();

#endif // SERVER_HPP
