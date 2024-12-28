#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <string>
#include <map>

class Response {
public:
    Response();

    void setStatusCode(int statusCode);
    void setContentType(const std::string& contentType);
    void setBody(const std::string& body);
    void setHeader(const std::string& key, const std::string& value);

    std::string build() const;

private:
    int _statusCode;
    std::string _contentType;
    std::string _body;
    std::map<std::string, std::string> _headers;

    std::string getStatusMessage(int code) const;
};

#endif // RESPONSE_HPP
