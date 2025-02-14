#include "request.hpp"
#include <sstream>
#include <algorithm>
#include <cctype>

static std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    size_t end = s.find_last_not_of(" \t\r\n");
    return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
}

Request::Request(const std::string& rawRequest) {
    parseRequest(rawRequest);
}

Request::~Request() {}

void Request::parseRequest(const std::string& rawRequest) {
    std::istringstream requestStream(rawRequest);
    std::string line;

    // Parsing della request line
    if (std::getline(requestStream, line)) {
        std::istringstream lineStream(line);
        lineStream >> _method;
        lineStream >> _path;
        size_t queryPos = _path.find('?');
        if (queryPos != std::string::npos) {
            _queryString = _path.substr(queryPos + 1);
            _path = _path.substr(0, queryPos);
        }
    }

    // Parsing degli header
    while (std::getline(requestStream, line)) {
        line = trim(line);
        if (line.empty())
            break;
        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos) {
            std::string key = trim(line.substr(0, colonPos));
            std::string value = trim(line.substr(colonPos + 1));
            _headers[key] = value;
        }
    }

    // Il corpo (body) della richiesta (se presente)
    std::ostringstream bodyStream;
    bodyStream << requestStream.rdbuf();
    _body = bodyStream.str();
}

std::string Request::getMethod() const {
    return _method;
}

std::string Request::getPath() const {
    return _path;
}

std::string Request::getQueryString() const {
    return _queryString;
}

std::string Request::getHeader(const std::string& key) const {
    std::map<std::string, std::string>::const_iterator it = _headers.find(key);
    return (it != _headers.end()) ? it->second : "";
}

std::string Request::getBody() const {
    return _body;
}
