#ifndef ERROR_HANDLER_HPP
#define ERROR_HANDLER_HPP

#include <string>

class ErrorHandler {
public:
    ErrorHandler() {}
    ~ErrorHandler() {}

    // Genera una risposta di errore formattata in base al codice di stato e al messaggio fornito
    std::string generateErrorResponse(int code, const std::string& message);
};

#endif // ERROR_HANDLER_HPP
