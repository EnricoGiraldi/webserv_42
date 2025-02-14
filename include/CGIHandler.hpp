#ifndef CGI_HANDLER_HPP
#define CGI_HANDLER_HPP

#include <string>
#include <map>

class CGIHandler {
public:
    CGIHandler();
    ~CGIHandler();

    // Esegue lo script CGI passando anche le variabili d'ambiente
    std::string executeCGI(const std::string& scriptPath, const std::string& queryString,
                           const std::map<std::string, std::string>& env);
};

#endif // CGI_HANDLER_HPP
