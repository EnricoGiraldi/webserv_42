#ifndef ROUTER_HPP
#define ROUTER_HPP

#include <string>
#include <map>

class Router {
public:
    // Funzione handler: restituisce una stringa e non riceve parametri
    typedef std::string (*HandlerFunction)(void);

    Router();
    ~Router();

    void addRoute(const std::string& path, HandlerFunction handler);
    HandlerFunction route(const std::string& path) const;

private:
    std::map<std::string, HandlerFunction> routes;
};

#endif // ROUTER_HPP
