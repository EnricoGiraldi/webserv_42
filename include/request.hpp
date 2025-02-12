#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <string>
#include <map>

class Request {
public:
    // Costruttore che accetta la richiesta raw come stringa
    Request(const std::string& rawRequest);

    // Distruttore
    ~Request();

    // Restituisce il metodo HTTP (GET, POST, etc.)
    std::string getMethod() const;

    // Restituisce il path della richiesta (URI)
    std::string getPath() const;

    // Restituisce la query string, se presente
    std::string getQueryString() const;

    // Restituisce il valore dell'header specificato
    std::string getHeader(const std::string& key) const;

private:
    std::string _method;
    std::string _path;
    std::string _queryString;
    std::map<std::string, std::string> _headers;

    // Metodo di parsing della richiesta raw
    void parseRequest(const std::string& rawRequest);
};

#endif // REQUEST_HPP
