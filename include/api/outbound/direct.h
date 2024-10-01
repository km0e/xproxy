#pragma once
#ifndef XPROXY_API_OUTBOUND_DIRECT
#  define XPROXY_API_OUTBOUND_DIRECT
#  include "api/outbound.h"

#  include <xsl/feature.h>

#  include <memory>
using namespace xsl;
using namespace xsl::feature;

class Direct : public Outbound {
public:
  Direct(std::string host, std::string port, std::shared_ptr<sync::Poller> poller)
      : Outbound(std::move(host), std::move(port)), poller(std::move(poller)) {}
  ~Direct() = default;
  coro::Task<std::tuple<std::size_t, std::optional<std::errc>>> read(
      std::span<OutConnection> buf) override {
    auto res
        = co_await tcp::dial<Tcp<Ip<4>>>(buf[0].ip.c_str(), buf[0].port.c_str(), *this->poller);
    if (!res) {
      co_return {0, std::errc::connection_refused};
    }

    auto [r, w] = std::move(*res).split();

    buf[0] = OutConnection{.ip = std::move(buf[0].ip),
                           .port = std::move(buf[0].port),
                           .in = std::move(r).to_unique_dyn(),
                           .out = std::move(w).to_unique_dyn()};

    co_return {1, std::nullopt};
  }

private:
  std::shared_ptr<sync::Poller> poller;
};
#endif
