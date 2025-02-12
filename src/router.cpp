#include "router.hpp"

// Definizione del costruttore (qui non è necessaria logica aggiuntiva)
Router::Router() {}

// Definizione del distruttore (qui non è necessaria logica aggiuntiva)
Router::~Router() {}

// Aggiunge una nuova route associando il percorso a una funzione handler.
void Router::addRoute(const std::string& path, HandlerFunction handler) {
    routes[path] = handler;
}

// Restituisce la funzione handler associata al percorso richiesto.
// Se il percorso non è presente, restituisce NULL.
Router::HandlerFunction Router::route(const std::string& path) const {
    std::map<std::string, HandlerFunction>::const_iterator it = routes.find(path);
    if (it != routes.end()) {
        return it->second;
    }
    return NULL;
}
