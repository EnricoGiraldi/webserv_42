#ifndef CONFIG_PARSER_HPP
#define CONFIG_PARSER_HPP

#include <string>
#include <map>
#include <fstream> // Necessario per std::ifstream

class ConfigParser {
public:
    // Ritorna true se il file di configurazione è stato parsato con successo
    bool parseConfigFile(const std::string& filePath);

    // Restituisce le impostazioni globali
    const std::map<std::string, std::string>& getGlobalSettings() const {
        return globalSettings;
    }

    // Restituisce le configurazioni delle location
    const std::map<std::string, std::map<std::string, std::string> >& getLocations() const {
        return locations;
    }

private:
    std::map<std::string, std::string> globalSettings;
    std::map<std::string, std::map<std::string, std::string> > locations;

    // Funzione helper per parsare un blocco di configurazione
    void parseBlock(std::ifstream& configFile, const std::string& blockName);
};

#endif // CONFIG_PARSER_HPP
