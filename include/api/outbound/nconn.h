#pragma once
#ifndef XPROXY_API_OUTBOUND_NCONN
#  define XPROXY_API_OUTBOUND_NCONN
#  include "api/outbound.h"
#  include "api/proto/nconn.h"

#  include <xsl/ai.h>
#  include <xsl/feature.h>
#  include <xsl/io.h>
#  include <xsl/net.h>

#  include <cstddef>
#  include <expected>
#  include <memory>
#  include <utility>
using namespace xsl;
using namespace xsl::feature;
class Nconn : public Outbound {
public:
  static std::unique_ptr<Nconn> make_unique(const std::shared_ptr<sync::Poller>& poller,
                                            std::string host, std::string port) {
    return std::make_unique<Nconn>(std::move(host), std::move(port), poller);
  }
  Nconn(std::string host, std::string port, std::shared_ptr<sync::Poller> poller)
      : Outbound(std::move(host), std::move(port)), poller(std::move(poller)) {}
  ~Nconn() = default;
  coro::Task<std::tuple<std::size_t, std::optional<std::errc>>> read(
      std::span<OutConnection> buf) override {
    auto res = co_await tcp::dial<Tcp<Ip<4>>>(this->host.c_str(), this->port.c_str(), *poller);
    INFO("Connecting to server");
    if (!res) {
      co_return {0, std::errc::connection_refused};
    }

    auto [r, w] = std::move(*res).split();

    NconnHeader header{buf[0].ip, buf[0].port};
    auto header_bytes = header.serialize();

    auto [_, err] = co_await w.write(std::as_bytes(std::span{header_bytes}));
    if (err) {
      co_return {0, err};
    }
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
