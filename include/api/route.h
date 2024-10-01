#pragma once
#ifndef XPROXY_API_ROUTE
#  define XPROXY_API_ROUTE
#  include "api/outbound.h"

#  include <memory>
#  include <string>
#  include <unordered_map>

class Route {
public:
  Route() = default;
  void add(std::shared_ptr<Outbound> out) { routes["test"] = out; }
  std::shared_ptr<Outbound> get(std::string host, std::string port) { return routes["test"]; }

protected:
  std::unordered_map<std::string, std::shared_ptr<Outbound>> routes;
};
#endif
