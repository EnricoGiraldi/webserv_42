#include "error_handler.hpp"
#include <sstream>

// Genera una risposta HTML di errore, utilizzata per restituire una pagina di errore al client.
std::string ErrorHandler::generateErrorResponse(int code, const std::string& message) {
    std::ostringstream oss;
    oss << "<html><body>"
        << "<h1>Error " << code << "</h1>"
        << "<p>" << message << "</p>"
        << "</body></html>";
    return oss.str();
}
