#pragma once
#ifndef XPROXY_API_INBOUND_HTTP
#  define XPROXY_API_INBOUND_HTTP
#  include "api/inbound.h"

#  include <xsl/ai.h>
#  include <xsl/feature.h>
#  include <xsl/io.h>
#  include <xsl/net.h>

#  include <cstddef>
#  include <expected>
#  include <memory>
#  include <string_view>
#  include <utility>
using namespace xsl;
using namespace xsl::feature;

const std::size_t HTTP_BUFFER_SIZE = 4096;

class HttpInbound : public Inbound {
public:
  using server_type = tcp::Server<Ip<4>>;
  static std::expected<std::unique_ptr<HttpInbound>, std::error_condition> make_unique(
      std::string host, std::string port, const std::shared_ptr<sync::Poller>& poller) {
    auto server = server_type::create(host.c_str(), port.c_str(), poller);
    if (!server) {
      return std::unexpected(server.error());
    }
    return std::make_unique<HttpInbound>(std::move(host), std::move(port), std::move(*server));
  }
  HttpInbound(std::string host, std::string port, tcp::Server<Ip<4>> server)
      : Inbound(std::move(host), std::move(port)), server(std::move(server)) {}
  ~HttpInbound() = default;
  coro::Task<std::tuple<std::size_t, std::optional<std::errc>>> read(
      std::span<InConnection> buf) override {
    auto arwd = co_await server.accept(nullptr);
    if (!arwd) {
      co_return {0, arwd.error()};
    }
    INFO("New connection request incoming");
    auto [r, w] = std::move(*arwd).split();
    http::Parser<> parser{};
    http::ParseData data{};
    auto parse_res = co_await parser.read(r, data);
    if (!parse_res) {
      co_return {0, parse_res.error()};
    }
    http::Request req{std::move(data.buffer), std::move(data.request), data.content_part, r};
    using in_dev_type = server_type::in_dev_type;
    using out_dev_type = server_type::out_dev_type;
    http::HandleContext<in_dev_type, out_dev_type> ctx{"", std::move(req)};
    if (req.method != http::Method::CONNECT) {
      ctx.easy_resp(http::Status::NOT_IMPLEMENTED);
      co_await ctx._response->sendto(w);
      co_return {0, std::errc::not_supported};
    }
    ctx.easy_resp(http::Status::OK);
    co_await ctx._response->sendto(w);

    auto colon = req.view.authority.find(':');
    auto host = std::string(req.view.authority.substr(0, colon));
    auto port = std::string(req.view.authority.substr(colon + 1));
    buf[0] = InConnection{.host = std::move(host),
                          .port = std::move(port),
                          .in = Splice<feature::In<std::byte>, feature::Dyn>::make_unique(
                              std::move(r).to_unique_dyn()),
                          .out = std::move(w).to_unique_dyn()};

    co_return {1, std::nullopt};
  }

private:
  tcp::Server<Ip<4>> server;
};

#endif
