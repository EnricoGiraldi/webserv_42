#include "router.hpp"

Router::Router() {}

Router::~Router() {}

void Router::addRoute(const std::string& path, HandlerFunction handler) {
    routes[path] = handler;
}

Router::HandlerFunction Router::route(const std::string& path) const {
    std::map<std::string, HandlerFunction>::const_iterator it = routes.find(path);
    return (it != routes.end()) ? it->second : NULL;
}

