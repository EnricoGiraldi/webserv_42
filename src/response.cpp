#include "response.hpp"
#include <sstream>

Response::Response() : _statusCode(200), _contentType("text/html"), _body("") {}

void Response::setStatusCode(int statusCode) {
    _statusCode = statusCode;
}

void Response::setContentType(const std::string& contentType) {
    _contentType = contentType;
}

void Response::setBody(const std::string& body) {
    _body = body;
}

void Response::setHeader(const std::string& key, const std::string& value) {
    _headers[key] = value;
}

std::string Response::build() const {
    std::ostringstream response;
    response << "HTTP/1.1 " << _statusCode << " " << getStatusMessage(_statusCode) << "\r\n"
             << "Content-Type: " << _contentType << "\r\n"
             << "Content-Length: " << _body.size() << "\r\n";

    for (std::map<std::string, std::string>::const_iterator it = _headers.begin(); it != _headers.end(); ++it) {
        response << it->first << ": " << it->second << "\r\n";
    }

    response << "\r\n" << _body;

    return response.str();
}

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
