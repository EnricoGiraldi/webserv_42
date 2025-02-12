#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <string>
#include <map>

class Response {
public:
    // Costruttore: inizializza lo status code e altri campi di default
    Response();

    // Imposta il codice di stato della risposta HTTP
    void setStatusCode(int statusCode);

    // Imposta il content type (ad esempio "text/html", "application/json", ecc.)
    void setContentType(const std::string& contentType);

    // Imposta il corpo della risposta (payload)
    void setBody(const std::string& body);

    // Imposta un header personalizzato per la risposta
    void setHeader(const std::string& key, const std::string& value);

    // Costruisce e restituisce la stringa completa della risposta HTTP
    std::string build() const;

private:
    int _statusCode;
    std::string _contentType;
    std::string _body;
    std::map<std::string, std::string> _headers;

    // Restituisce il messaggio associato ad un determinato codice di stato (es. "OK" per 200)
    std::string getStatusMessage(int code) const;
};

#endif // RESPONSE_HPP
