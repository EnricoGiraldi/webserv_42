#include "server.hpp"
#include "CGIHandler.hpp"
#include "error_handler.hpp"
#include "router.hpp"
#include "request.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <poll.h>
#include <vector>
#include <dirent.h>
#include <sys/stat.h>
#include <map>

// Istanza globale del router
Router router;

// Funzione di esempio per il routing.
std::string exampleHandler() {
    return "<html><body><h1>Example Page</h1></body></html>";
}

Server::Server(const ConfigParser& configParser) : _isRunning(false) {
    // Usa le impostazioni globali del file di configurazione
    const std::map<std::string, std::string>& globals = configParser.getGlobalSettings();
    _port = globals.count("port") ? std::atoi(globals.at("port").c_str()) : 8080;
    _documentRoot = globals.count("root") ? globals.at("root") : "www";

    setupSocket(_port);

    // Aggiunge una route di esempio
    router.addRoute("/example", exampleHandler);
}

Server::~Server() {
    stop();
}

void Server::setupSocket(int port) {
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

    if (bind(_serverSocket, reinterpret_cast<struct sockaddr*>(&serverAddr), sizeof(serverAddr)) < 0) {
        close(_serverSocket);
        throw std::runtime_error("Failed to bind socket to port.");
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

    std::vector<pollfd> fds;
    pollfd serverFd = { _serverSocket, POLLIN, 0 };
    fds.push_back(serverFd);

    while (_isRunning) {
        int activity = poll(fds.data(), fds.size(), -1);
        if (activity < 0) {
            std::cerr << "Poll error." << std::endl;
            continue;
        }

        for (size_t i = 0; i < fds.size(); ++i) {
            if (fds[i].revents & POLLIN) {
                if (fds[i].fd == _serverSocket) {
                    sockaddr_in clientAddr;
                    socklen_t clientLen = sizeof(clientAddr);
                    int clientSocket = accept(_serverSocket, reinterpret_cast<struct sockaddr*>(&clientAddr), &clientLen);
                    if (clientSocket >= 0) {
                        pollfd clientFd = { clientSocket, POLLIN, 0 };
                        fds.push_back(clientFd);
                    }
                } else {
                    handleClient(fds[i].fd);
                    close(fds[i].fd);
                    fds.erase(fds.begin() + i);
                    --i;
                }
            }
        }
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
    char buffer[4096];
    memset(buffer, 0, sizeof(buffer));

    ssize_t bytesRead = read(clientSocket, buffer, sizeof(buffer) - 1);
    if (bytesRead < 0) {
        std::cerr << "Failed to read from client." << std::endl;
        return;
    }

    std::string rawRequest(buffer);
    Request request(rawRequest);

    std::string method = request.getMethod();
    std::string path = request.getPath();
    std::string query = request.getQueryString();
    std::string body = request.getBody();

    if (path.empty()) {
        sendErrorResponse(clientSocket, 400, "Invalid Request");
        return;
    }

    // Se il path inizia per /cgi-bin/, gestisce la richiesta CGI
    if (path.find("/cgi-bin/") == 0) {
        handleCGIRequest(clientSocket, path, query);
        return;
    }

    if (method == "GET") {
        serveStaticFile(clientSocket, path);
    } else if (method == "POST") {
        handlePostRequest(clientSocket, body);
    } else if (method == "DELETE") {
        std::string fullPath = _documentRoot + path;
        if (access(fullPath.c_str(), F_OK) == -1) {
            sendErrorResponse(clientSocket, 404, "File Not Found");
            return;
        }
        if (remove(fullPath.c_str()) == 0) {
            sendResponse(clientSocket, 200, "text/plain", "Resource deleted successfully");
        } else {
            sendErrorResponse(clientSocket, 500, "Internal Server Error: Unable to delete file");
        }
    } else {
        sendErrorResponse(clientSocket, 405, "Method Not Allowed");
    }
}

void Server::handlePostRequest(int clientSocket, const std::string& body) {
    // Log dei dati POST
    std::ofstream logFile("post_data.log", std::ios::app);
    if (logFile.is_open()) {
        logFile << "POST data: " << body << std::endl;
        logFile.close();
    }

    std::string responseContent =
        "<!DOCTYPE html>\n"
        "<html>\n"
        "<head><meta charset=\"UTF-8\"><title>Dati Ricevuti</title></head>\n"
        "<body>\n"
        "<h1>Dati Ricevuti</h1>\n"
        "<p>I dati inviati sono: " + body + "</p>\n"
        "<a href=\"/\">Torna alla Home</a>\n"
        "</body>\n"
        "</html>";

    sendResponse(clientSocket, 200, "text/html", responseContent);
}

void Server::handleCGIRequest(int clientSocket, const std::string& path, const std::string& query) {
    // Assume che lo script CGI si trovi in _documentRoot + path
    std::string scriptPath = _documentRoot + path;
    CGIHandler cgiHandler;
    // Ambiente minimo per il CGI (estendibile se necessario)
    std::map<std::string, std::string> env;
    env["REQUEST_METHOD"] = "GET"; // oppure POST se necessario
    env["QUERY_STRING"] = query;
    std::string cgiOutput = cgiHandler.executeCGI(scriptPath, query, env);
    sendResponse(clientSocket, 200, "text/html", cgiOutput);
}

void Server::serveStaticFile(int clientSocket, const std::string& path) {
    std::string filePath = _documentRoot + path;
    struct stat path_stat;
    if (stat(filePath.c_str(), &path_stat) == 0 && S_ISDIR(path_stat.st_mode)) {
        std::string indexPath = filePath;
        if (!indexPath.empty() && indexPath[indexPath.size() - 1] != '/')
            indexPath += "/";
        indexPath += "index.html";

        if (stat(indexPath.c_str(), &path_stat) == 0) {
            std::ifstream file(indexPath.c_str(), std::ios::binary);
            if (file) {
                std::ostringstream content;
                content << file.rdbuf();
                sendResponse(clientSocket, 200, "text/html", content.str());
                return;
            }
        }
        // Autoindex (directory listing)
        std::ostringstream response;
        response << "<html><body><h1>Index of " << path << "</h1><ul>";
        DIR* dir = opendir(filePath.c_str());
        if (dir) {
            struct dirent* entry;
            while ((entry = readdir(dir)) != NULL) {
                std::string entryName(entry->d_name);
                if (entryName == "." || entryName == "..")
                    continue;
                response << "<li><a href=\"" << entryName << "\">" << entryName << "</a></li>";
            }
            closedir(dir);
        }
        response << "</ul></body></html>";
        sendResponse(clientSocket, 200, "text/html", response.str());
        return;
    }

    std::ifstream file(filePath.c_str(), std::ios::binary);
    if (!file) {
        sendErrorResponse(clientSocket, 404, "File Not Found");
        return;
    }
    std::ostringstream content;
    content << file.rdbuf();
    std::string mimeType = detectMimeType(filePath);
    sendResponse(clientSocket, 200, mimeType, content.str());
}

std::string Server::detectMimeType(const std::string& path) {
    size_t dotPos = path.find_last_of(".");
    if (dotPos != std::string::npos) {
        std::string ext = path.substr(dotPos + 1);
        if (ext == "html" || ext == "htm")
            return "text/html";
        if (ext == "css")
            return "text/css";
        if (ext == "js")
            return "application/javascript";
        if (ext == "png")
            return "image/png";
        if (ext == "jpg" || ext == "jpeg")
            return "image/jpeg";
        if (ext == "gif")
            return "image/gif";
    }
    return "text/plain";
}

void Server::sendResponse(int clientSocket, int statusCode, const std::string& contentType, const std::string& content) {
    std::ostringstream response;
    response << "HTTP/1.1 " << statusCode << " ";
    switch (statusCode) {
        case 200: response << "OK"; break;
        case 201: response << "Created"; break;
        case 400: response << "Bad Request"; break;
        case 403: response << "Forbidden"; break;
        case 404: response << "Not Found"; break;
        case 405: response << "Method Not Allowed"; break;
        case 500: response << "Internal Server Error"; break;
        default:  response << "Unknown"; break;
    }
    response << "\r\n";
    response << "Content-Length: " << content.size() << "\r\n";
    response << "Content-Type: " << contentType << "\r\n";
    response << "Connection: close\r\n";
    response << "\r\n" << content;
    std::string responseStr = response.str();
    send(clientSocket, responseStr.c_str(), responseStr.size(), 0);
}

void Server::sendErrorResponse(int clientSocket, int statusCode, const std::string& message) {
    std::ostringstream response;
    response << "<html><body><h1>Error " << statusCode << "</h1><p>" << message << "</p></body></html>";
    sendResponse(clientSocket, statusCode, "text/html", response.str());
}

std::pair<std::string, std::string> Server::parseRequest(const std::string& request) {
    size_t methodEnd = request.find(" ");
    if (methodEnd == std::string::npos)
        return std::make_pair("", "");
    std::string method = request.substr(0, methodEnd);
    size_t pathStart = methodEnd + 1;
    size_t pathEnd = request.find(" ", pathStart);
    if (pathEnd == std::string::npos)
        return std::make_pair("", "");
    std::string path = request.substr(pathStart, pathEnd - pathStart);
    return std::make_pair(method, path);
}
