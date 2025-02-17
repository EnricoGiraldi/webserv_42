// src/server.cpp

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
#include <ctime>
#include <stdlib.h> // per strtoul

// Istanza globale del router
Router router;

// Funzione di esempio per il routing
std::string exampleHandler() {
    return "<html><body><h1>Example Page</h1></body></html>";
}

// Funzione helper: invia tutti i byte presenti in buf
static int send_all(int sockfd, const char* buf, size_t len) {
    size_t total = 0;
    while (total < len) {
        ssize_t sent = send(sockfd, buf + total, len - total, 0);
        if (sent < 0) {
            perror("send_all");
            return -1;
        }
        if (sent == 0) {
            break; // connessione chiusa
        }
        total += sent;
    }
    return total;
}

// Costruttore: inizializza il server usando la configurazione
Server::Server(const ConfigParser& configParser) : _isRunning(false) {
    const std::map<std::string, std::string>& globals = configParser.getGlobalSettings();
    _port = globals.count("port") ? std::atoi(globals.at("port").c_str()) : 8080;
    _documentRoot = globals.count("root") ? globals.at("root") : "www";
    setupSocket(_port);
    // Aggiunge una route di esempio
    router.addRoute("/example", exampleHandler);
}

// Distruttore
Server::~Server() {
    stop();
}

// Configura il socket del server
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

// Loop principale del server
void Server::run() {
    _isRunning = true;
    std::cout << "Server is running..." << std::endl;
    std::vector<pollfd> fds;
    pollfd serverFd = { _serverSocket, POLLIN, 0 };
    fds.push_back(serverFd);
    const int TIMEOUT_MS = 5000; // Timeout di 5 secondi
    while (_isRunning) {
        int activity = poll(fds.data(), fds.size(), TIMEOUT_MS);
        if (activity < 0) {
            std::cerr << "Poll error." << std::endl;
            continue;
        }
        if (activity == 0)
            continue;
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

// Ferma il server
void Server::stop() {
    if (_isRunning) {
        _isRunning = false;
        close(_serverSocket);
        std::cout << "Server stopped." << std::endl;
    }
}

// Gestisce la richiesta di un client
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

    // Validazione degli header: controllo del campo "Host"
    if (request.getHeader("Host").empty()) {
        sendErrorResponse(clientSocket, 400, "Bad Request: Missing Host header");
        return;
    }
    // Log del User-Agent (se presente)
    std::string userAgent = request.getHeader("User-Agent");
    if (!userAgent.empty()) {
        std::cout << "User-Agent: " << userAgent << std::endl;
    }
    // Redirezione: se il path è "/old-page", redirigi a "/new-page"
    if (path == "/old-page") {
        std::ostringstream oss;
        oss << "HTTP/1.1 301 Moved Permanently\r\n"
            << "Location: /new-page\r\n"
            << "Content-Length: 0\r\n"
            << "Connection: close\r\n\r\n";
        std::string responseStr = oss.str();
        send_all(clientSocket, responseStr.data(), responseStr.size());
        return;
    }
    // Blocca l'accesso a directory specifiche, ad esempio "/blocked"
    if (path.find("/blocked") == 0) {
        sendErrorResponse(clientSocket, 403, "Forbidden: Access to this directory is blocked");
        return;
    }
    bool isHead = (method == "HEAD");
    // Se il path inizia per "/cgi-bin/", gestisci la richiesta CGI
    if (path.find("/cgi-bin/") == 0) {
        handleCGIRequest(clientSocket, path, query);
        return;
    }
    // Gestione POST
    if (method == "POST") {
        // Se è una richiesta di upload
        if (path == "/upload") {
            std::string contentLengthStr = request.getHeader("Content-Length");
            if (contentLengthStr.empty()) {
                sendErrorResponse(clientSocket, 400, "Bad Request: Missing Content-Length header");
                return;
            }
            size_t contentLength = strtoul(contentLengthStr.c_str(), NULL, 10);
            if (contentLength > MAX_UPLOAD_SIZE) {
                sendErrorResponse(clientSocket, 413, "Payload Too Large");
                return;
            }
            std::string uploadDir = "uploads/";
            system(("mkdir -p " + uploadDir).c_str());
            std::ostringstream filename;
            filename << uploadDir << "upload_" << time(NULL) << ".dat";
            std::ofstream file(filename.str().c_str(), std::ios::binary);
            if (!file) {
                sendErrorResponse(clientSocket, 403, "Forbidden: Cannot write to upload directory");
                return;
            }
            file << body;
            file.close();
            // Per POST, invia sempre il body, quindi il flag è true
            sendResponse(clientSocket, 201, "text/plain", "File uploaded successfully", true);
            return;
        } else {
            // POST normale: log dei dati e risposta HTML
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
            // Per POST, invia sempre il body (true)
            sendResponse(clientSocket, 200, "text/html", responseContent, true);
            return;
        }
    }
    // Gestione GET e HEAD
    if (method == "GET" || method == "HEAD") {
        // Per GET, isHead è false, quindi !isHead è true; per HEAD, isHead è true, quindi !isHead è false.
        serveStaticFile(clientSocket, path, isHead);
        return;
    }
    // Metodo non supportato
    sendErrorResponse(clientSocket, 405, "Method Not Allowed");
}

// Gestisce la richiesta CGI
void Server::handleCGIRequest(int clientSocket, const std::string& path, const std::string& query) {
    std::string scriptPath = _documentRoot + path;
    CGIHandler cgiHandler;
    std::map<std::string, std::string> env;
    env["REQUEST_METHOD"] = "GET"; // oppure "POST" se necessario
    env["QUERY_STRING"] = query;
    std::string cgiOutput = cgiHandler.executeCGI(scriptPath, query, env);
    sendResponse(clientSocket, 200, "text/html", cgiOutput, true);
}

// Serve file statici: implementazione unica
void Server::serveStaticFile(int clientSocket, const std::string& path, bool isHead) {
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
                std::string body = content.str();
                std::cout << "Read index.html, size = " << body.size() << " bytes." << std::endl;
                // Per GET, invia il body; per HEAD, non invia il body
                sendResponse(clientSocket, 200, "text/html", body, !isHead);
                return;
            }
        }
        // Autoindex: genera una lista dei file nella directory
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
        std::string body = response.str();
        sendResponse(clientSocket, 200, "text/html", body, !isHead);
        return;
    }
    std::ifstream file(filePath.c_str(), std::ios::binary);
    if (!file) {
        sendErrorResponse(clientSocket, 404, "File Not Found");
        return;
    }
    std::ostringstream content;
    content << file.rdbuf();
    std::string body = content.str();
    std::cout << "Read file " << filePath << ", size = " << body.size() << " bytes." << std::endl;
    std::string mimeType = detectMimeType(filePath);
    sendResponse(clientSocket, 200, mimeType, body, !isHead);
}

// Determina il MIME type in base all'estensione del file
std::string Server::detectMimeType(const std::string& path) {
    size_t dotPos = path.find_last_of(".");
    if (dotPos != std::string::npos) {
        std::string ext = path.substr(dotPos + 1); // Dichiarazione di "ext"
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


// Invia la risposta completa usando la funzione helper send_all
void Server::sendResponse(int clientSocket, int statusCode, const std::string& contentType,
                          const std::string& body, bool sendBody) {
    std::ostringstream oss;
    oss << "HTTP/1.1 " << statusCode << " ";
    switch (statusCode) {
        case 200: oss << "OK"; break;
        case 201: oss << "Created"; break;
        case 301: oss << "Moved Permanently"; break;
        case 302: oss << "Found"; break;
        case 400: oss << "Bad Request"; break;
        case 403: oss << "Forbidden"; break;
        case 404: oss << "Not Found"; break;
        case 405: oss << "Method Not Allowed"; break;
        case 413: oss << "Payload Too Large"; break;
        case 500: oss << "Internal Server Error"; break;
        default: oss << "Unknown"; break;
    }
    oss << "\r\n";
    oss << "Content-Length: " << body.size() << "\r\n";
    oss << "Content-Type: " << contentType << "\r\n";
    oss << "Connection: close\r\n";
    oss << "\r\n";
    if (sendBody)
        oss << body;
    std::string responseStr = oss.str();
    std::cout << "Sending response: total size = " << responseStr.size() << " bytes." << std::endl;
    int ret = send_all(clientSocket, responseStr.data(), responseStr.size());
    if (ret < 0) {
        std::cerr << "Error during send_all" << std::endl;
    } else {
        std::cout << "Total sent: " << ret << " bytes." << std::endl;
    }
}

// Invia una risposta di errore, cercando una pagina d'errore personalizzata
void Server::sendErrorResponse(int clientSocket, int statusCode, const std::string& message) {
    std::ostringstream oss;
    oss << statusCode;
    std::string codeStr = oss.str();
    std::string errorPagePath = _documentRoot + "/errors/" + codeStr + ".html";
    std::ifstream errorFile(errorPagePath.c_str(), std::ios::binary);
    std::string content;
    if (errorFile) {
        std::ostringstream ss;
        ss << errorFile.rdbuf();
        content = ss.str();
    } else {
        std::ostringstream ss;
        ss << "<html><body><h1>Error " << statusCode << "</h1><p>" << message << "</p></body></html>";
        content = ss.str();
    }
    sendResponse(clientSocket, statusCode, "text/html", content, true);
}
