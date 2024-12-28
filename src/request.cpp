#include "request.hpp"
#include <sstream>

Request::Request(const std::string& rawRequest) {
    parseRequest(rawRequest);
}

Request::~Request() {}

void Request::parseRequest(const std::string& rawRequest) {
    std::istringstream requestStream(rawRequest);
    std::string line;

    // Parse the request line
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

    // Parse headers
    while (std::getline(requestStream, line) && line != "\r") {
        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos) {
            std::string key = line.substr(0, colonPos);
            std::string value = line.substr(colonPos + 2); // Skip ": "
            _headers[key] = value;
        }
    }
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
    if (_headers.find(key) != _headers.end()) {
        return _headers.at(key);
    }
    return "";
}
