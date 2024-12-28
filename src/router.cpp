#include "router.hpp"

Router::Router() {}

Router::~Router() {}

void Router::addRoute(const std::string& path, HandlerFunction handler) {
    _routes[path] = handler;
}

Router::HandlerFunction Router::route(const std::string& path) const {
    std::map<std::string, HandlerFunction>::const_iterator it = _routes.find(path);
    if (it != _routes.end()) {
        return it->second;
    }
    return NULL; // No handler found
}
