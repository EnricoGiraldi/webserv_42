#include "response.hpp"
#include <sstream>

// Costruttore: inizializza lo status code a 200, il content type a "text/html" e il body vuoto.
Response::Response() : _statusCode(200), _contentType("text/html"), _body("") {}

// Imposta il codice di stato della risposta.
void Response::setStatusCode(int statusCode) {
    _statusCode = statusCode;
}

// Imposta il Content-Type della risposta.
void Response::setContentType(const std::string& contentType) {
    _contentType = contentType;
}

// Imposta il corpo (body) della risposta.
void Response::setBody(const std::string& body) {
    _body = body;
}

// Imposta o aggiorna un header personalizzato della risposta.
void Response::setHeader(const std::string& key, const std::string& value) {
    _headers[key] = value;
}

// Costruisce la stringa completa della risposta HTTP da inviare al client.
std::string Response::build() const {
    std::ostringstream response;

    // Costruisce la status line della risposta HTTP.
    response << "HTTP/1.1 " << _statusCode << " " << getStatusMessage(_statusCode) << "\r\n";

    // Aggiunge i campi di header standard.
    response << "Content-Type: " << _contentType << "\r\n";
    response << "Content-Length: " << _body.size() << "\r\n";

    // Aggiunge gli header personalizzati, se presenti.
    for (std::map<std::string, std::string>::const_iterator it = _headers.begin(); it != _headers.end(); ++it) {
        response << it->first << ": " << it->second << "\r\n";
    }

    // Separa gli header dal corpo della risposta.
    response << "\r\n" << _body;

    return response.str();
}

// Restituisce il messaggio associato al codice di stato HTTP.
std::string Response::getStatusMessage(int code) const {
    switch (code) {
        case 200: return "OK";
        case 400: return "Bad Request";
        case 403: return "Forbidden";
        case 404: return "Not Found";
        case 500: return "Internal Server Error";
        default:  return "Unknown Status";
    }
}
