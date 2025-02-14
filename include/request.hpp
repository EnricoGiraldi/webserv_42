#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <string>
#include <map>

class Request {
public:
    // Costruttore che parsifica la richiesta raw
    Request(const std::string& rawRequest);
    ~Request();

    std::string getMethod() const;
    std::string getPath() const;
    std::string getQueryString() const;
    std::string getHeader(const std::string& key) const;
    std::string getBody() const;

private:
    std::string _method;
    std::string _path;
    std::string _queryString;
    std::map<std::string, std::string> _headers;
    std::string _body;

    void parseRequest(const std::string& rawRequest);
};

#endif // REQUEST_HPP
