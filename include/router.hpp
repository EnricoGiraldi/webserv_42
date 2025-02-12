#ifndef ROUTER_HPP
#define ROUTER_HPP

#include <string>
#include <map>

class Router {
public:
    // Tipo per la funzione handler: una funzione che restituisce std::string e non riceve parametri.
    typedef std::string (*HandlerFunction)();

    Router();
    ~Router();

    // Aggiunge una route associando un percorso a una funzione handler.
    void addRoute(const std::string& path, HandlerFunction handler);
    // Restituisce la funzione handler associata al percorso richiesto.
    HandlerFunction route(const std::string& path) const;

private:
    // Mappa che associa un percorso a una funzione handler.
    std::map<std::string, HandlerFunction> routes;
};

#endif // ROUTER_HPP
