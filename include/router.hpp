#ifndef ROUTER_HPP
#define ROUTER_HPP

#include <map>
#include <string>

class Router {
public:
    typedef std::string (*HandlerFunction)(); // Define a function pointer type for handlers

    Router();
    ~Router();

    void addRoute(const std::string& path, HandlerFunction handler);
    HandlerFunction route(const std::string& path) const;

private:
    std::map<std::string, HandlerFunction> _routes; // Store paths and handlers
};

#endif // ROUTER_HPP
