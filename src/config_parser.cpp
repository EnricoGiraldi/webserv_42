#include "config_parser.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <algorithm>

bool ConfigParser::parseConfigFile(const std::string& filePath) {
    std::ifstream configFile(filePath.c_str());
    if (!configFile.is_open()) {
        std::cerr << "Failed to open config file: " << filePath << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(configFile, line)) {
        // Rimuovi gli spazi dalla linea.
        // Attenzione: questa operazione rimuove tutti gli spazi e potrebbe influire sui valori
        // se questi devono contenere spazi. Valuta se è necessario un trim (rimuovere solo spazi iniziali e finali).
        line.erase(std::remove(line.begin(), line.end(), ' '), line.end());

        // Salta le righe vuote o i commenti (linee che iniziano con '#')
        if (line.empty() || line[0] == '#') {
            continue;
        }

        // Cerca l'inizio di un blocco "server {"
        if (line.find("server{") != std::string::npos) {
            parseBlock(configFile, "server");
        }
    }

    return true;
}

void ConfigParser::parseBlock(std::ifstream& configFile, const std::string& /* blockName */) {
    std::string line;
    std::string currentLocation;

    while (std::getline(configFile, line)) {
        // Rimuove tutti gli spazi dalla linea
        line.erase(std::remove(line.begin(), line.end(), ' '), line.end());

        // Se troviamo la parentesi graffa di chiusura, esci dal blocco
        if (line.find("}") != std::string::npos) {
            break;
        }

        // Trova il separatore '=' per dividere key e value
        size_t separator = line.find('=');
        if (separator != std::string::npos) {
            std::string key = line.substr(0, separator);
            std::string value = line.substr(separator + 1);

            // Se la chiave è "location", inizializza una nuova location
            if (key == "location") {
                currentLocation = value;
                locations[currentLocation] = std::map<std::string, std::string>();
            }
            // Se siamo all'interno di un blocco location, aggiungi l'impostazione alla location corrente
            else if (!currentLocation.empty()) {
                locations[currentLocation][key] = value;
            }
            // Altrimenti, la impostazione è globale
            else {
                globalSettings[key] = value;
            }
        }
    }
}
