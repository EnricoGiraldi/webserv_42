#ifndef ERROR_HANDLER_HPP
#define ERROR_HANDLER_HPP

#include <string>

class ErrorHandler {
public:
    ErrorHandler();
    ~ErrorHandler();

    // Genera una pagina HTML d'errore basata sul codice e messaggio
    std::string generateErrorResponse(int code, const std::string& message);
};

#endif // ERROR_HANDLER_HPP
