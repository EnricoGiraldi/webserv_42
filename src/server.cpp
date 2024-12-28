#include "server.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <map>
#include "cgi_handler.hpp"
#include "error_handler.hpp"
#include "router.hpp"

// Global router instance
Router router;

Server::Server(const Config& config) : _isRunning(false) {
    setupSocket(config);

    // Add predefined routes
    router.addRoute("/example", exampleHandler);
}

Server::~Server() {
    stop();
}

void Server::setupSocket(const Config& config) {
    int port = 8080; // Default port
    if (config.global_settings.find("port") != config.global_settings.end()) {
        port = std::atoi(config.global_settings.at("port").c_str());
    }

    _serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (_serverSocket < 0) {
        throw std::runtime_error("Failed to create socket.");
    }

    int opt = 1;
    if (setsockopt(_serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        close(_serverSocket);
        throw std::runtime_error("Failed to set socket options.");
    }

    sockaddr_in serverAddr = {};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(_serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        close(_serverSocket);
        std::ostringstream oss;
        oss << "Failed to bind socket to port " << port;
        throw std::runtime_error(oss.str());
    }

    if (listen(_serverSocket, 10) < 0) {
        close(_serverSocket);
        throw std::runtime_error("Failed to listen on socket.");
    }

    std::cout << "Server is listening on port " << port << "." << std::endl;
}

void Server::run() {
    _isRunning = true;
    std::cout << "Server is running..." << std::endl;

    while (_isRunning) {
        sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);
        int clientSocket = accept(_serverSocket, (struct sockaddr*)&clientAddr, &clientLen);

        if (clientSocket < 0) {
            std::cerr << "Failed to accept client." << std::endl;
            continue;
        }

        handleClient(clientSocket);
        close(clientSocket);
    }
}

void Server::stop() {
    if (_isRunning) {
        _isRunning = false;
        close(_serverSocket);
        std::cout << "Server stopped." << std::endl;
    }
}

void Server::handleClient(int clientSocket) {
    char buffer[2048]; // Increase buffer size to handle larger POST requests
    memset(buffer, 0, sizeof(buffer));

    ssize_t bytesRead = read(clientSocket, buffer, sizeof(buffer) - 1);
    if (bytesRead < 0) {
        std::cerr << "Failed to read from client." << std::endl;
        return;
    }

    logRequest(buffer);
    std::string request(buffer);

    // Parse HTTP method and path
    std::pair<std::string, std::string> requestInfo = parseRequest(request);
    std::string method = requestInfo.first;
    std::string path = requestInfo.second;

    if (path.empty()) {
        sendErrorResponse(clientSocket, 400, "Invalid Request");
        return;
    }

    if (method == "GET") {
        Router::HandlerFunction handler = router.route(path);
        if (handler != NULL) {
            sendResponse(clientSocket, 200, "text/html", handler());
            return;
        }

        if (path.find("/cgi-bin/") == 0) {
            handleCGIRequest(clientSocket, path);
            return;
        }

        serveStaticFile(clientSocket, path);
    } else if (method == "POST") {
        // Extract body data from request
        size_t bodyStart = request.find("\r\n\r\n");
        if (bodyStart == std::string::npos) {
            sendErrorResponse(clientSocket, 400, "Bad Request");
            return;
        }
        std::string body = request.substr(bodyStart + 4);

        // Log received data for debugging
        std::ofstream logFile("post_data.log", std::ios::app);
        if (logFile) {
            logFile << "POST Data: " << body << std::endl;
        }

        // Send response back with echoed data
        std::string responseContent = "Received POST data: " + body;
        sendResponse(clientSocket, 200, "text/plain", responseContent);
    } else if (method == "DELETE") {
        // Simulate DELETE handling
        sendResponse(clientSocket, 200, "text/plain", "Resource deleted (simulated)");
    } else {
        sendErrorResponse(clientSocket, 405, "Method Not Allowed");
    }
}

std::pair<std::string, std::string> Server::parseRequest(const std::string& request) {
    size_t methodEnd = request.find(" ");
    if (methodEnd == std::string::npos) {
        return std::make_pair("", "");
    }

    std::string method = request.substr(0, methodEnd);
    size_t pathStart = methodEnd + 1;
    size_t pathEnd = request.find(" ", pathStart);

    if (pathEnd == std::string::npos) {
        return std::make_pair("", "");
    }

    std::string path = request.substr(pathStart, pathEnd - pathStart);
    if (path == "/") {
        path = "/index.html";
    }

    return std::make_pair(method, path);
}

void Server::logRequest(const char* buffer) {
    std::ofstream logFile("server.log", std::ios::app);
    if (logFile) {
        logFile << "Received request:\n" << buffer << "\n\n";
    }
}

void Server::serveStaticFile(int clientSocket, const std::string& path) {
    std::string filePath = "www" + path;
    std::ifstream file(filePath.c_str());

    if (file) {
        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        std::string mimeType = detectMimeType(path);

        sendResponse(clientSocket, 200, mimeType, content);
    } else {
        sendErrorResponse(clientSocket, 404, "The requested file was not found.");
    }
}

std::string Server::detectMimeType(const std::string& path) {
    std::map<std::string, std::string> mimeTypes;
    mimeTypes[".html"] = "text/html";
    mimeTypes[".css"] = "text/css";
    mimeTypes[".js"] = "application/javascript";
    mimeTypes[".png"] = "image/png";
    mimeTypes[".jpg"] = "image/jpeg";
    mimeTypes[".gif"] = "image/gif";
    mimeTypes[".ico"] = "image/x-icon";
    mimeTypes[".json"] = "application/json";
    mimeTypes[".xml"] = "application/xml";
    mimeTypes[".txt"] = "text/plain";

    size_t dotPos = path.rfind('.');
    if (dotPos != std::string::npos) {
        std::string extension = path.substr(dotPos);
        std::map<std::string, std::string>::iterator it = mimeTypes.find(extension);
        if (it != mimeTypes.end()) {
            return it->second;
        }
    }

    return "application/octet-stream"; // Default MIME type
}

void Server::handleCGIRequest(int clientSocket, const std::string& path) {
    std::map<std::string, std::string> env;
    env["QUERY_STRING"] = "";
    env["SCRIPT_NAME"] = path;
    env["REQUEST_METHOD"] = "GET";
    env["SERVER_PROTOCOL"] = "HTTP/1.1";

    try {
        CGIHandler cgi;
        std::string cgiOutput = cgi.executeCGI("www" + path, "", env);
        sendResponse(clientSocket, 200, "text/html", cgiOutput);
    } catch (const std::exception& e) {
        sendErrorResponse(clientSocket, 500, e.what());
    }
}

void Server::sendResponse(int clientSocket, int statusCode, const std::string& contentType, const std::string& content) {
    std::ostringstream response;
    response << "HTTP/1.1 " << statusCode << " " << getStatusMessage(statusCode) << "\r\n"
             << "Content-Length: " << content.size() << "\r\n"
             << "Content-Type: " << contentType << "\r\n"
             << "Connection: close\r\n"
             << "\r\n"
             << content;

    std::string responseStr = response.str();
    send(clientSocket, responseStr.c_str(), responseStr.size(), 0);
}

std::string Server::getStatusMessage(int statusCode) {
    switch (statusCode) {
        case 200: return "OK";
        case 400: return "Bad Request";
        case 404: return "Not Found";
        case 405: return "Method Not Allowed";
        case 500: return "Internal Server Error";
        default: return "Unknown Status";
    }
}

void Server::sendErrorResponse(int clientSocket, int statusCode, const std::string& message) {
    ErrorHandler errorHandler;
    std::string errorResponse = errorHandler.generateErrorResponse(statusCode, message);
    sendResponse(clientSocket, statusCode, "text/html", errorResponse);
}

std::string exampleHandler() {
    return "<html><body><h1>Example Page</h1></body></html>";
}

