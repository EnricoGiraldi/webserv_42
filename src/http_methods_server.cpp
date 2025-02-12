#include "server.hpp"
#include <iostream>
#include <fstream>
#include <unistd.h>

// Implementazione della gestione della richiesta POST
void Server::handlePostRequest(int clientSocket, const std::string& body) {
    // Log dei dati ricevuti
    std::ofstream logFile("post_data.log", std::ios::app);
    if (logFile.is_open()) {
        logFile << "POST data: " << body << std::endl;
        logFile.close();
    }

    // Costruisci una risposta HTML più gradevole
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

    // Invia la risposta con Content-Type "text/html"
    sendResponse(clientSocket, 200, "text/html", responseContent);
}


// Implementazione della gestione della richiesta DELETE
void Server::handleDeleteRequest(int clientSocket, const std::string& path) {
    // Simula la cancellazione della risorsa e prepara la risposta
    std::string responseContent = "Resource at " + path + " deleted (simulated)";
    sendResponse(clientSocket, 200, "text/plain", responseContent);
}
