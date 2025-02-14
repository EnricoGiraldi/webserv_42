#ifndef CONFIG_PARSER_HPP
#define CONFIG_PARSER_HPP

#include <string>
#include <map>

class ConfigParser {
public:
    ConfigParser();
    ~ConfigParser();

    // Restituisce true se il file di configurazione è stato parsato con successo
    bool parseConfigFile(const std::string& filePath);

    // Impostazioni globali (es. port, root, ecc.)
    const std::map<std::string, std::string>& getGlobalSettings() const;

    // Configurazioni per location: chiave = location, valore = mappa di impostazioni
    const std::map<std::string, std::map<std::string, std::string> >& getLocations() const;

private:
    std::map<std::string, std::string> globalSettings;
    std::map<std::string, std::map<std::string, std::string> > locations;

    void parseBlock(std::istream& configStream, const std::string& blockName);
    std::string trim(const std::string& s);
};

#endif // CONFIG_PARSER_HPP
