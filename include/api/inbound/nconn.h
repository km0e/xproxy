#pragma once
#ifndef XPROXY_API_INBOUND_NCONN
#  define XPROXY_API_INBOUND_NCONN
#  include "api/inbound.h"
#  include "api/proto/nconn.h"

#  include <xsl/ai.h>
#  include <xsl/feature.h>
#  include <xsl/io.h>
#  include <xsl/net.h>
#  include <xsl/sys/net/io.h>

#  include <cstddef>
#  include <expected>
#  include <memory>
#  include <utility>

using namespace xsl;
using namespace xsl::feature;

class ConnectionWithHeader : public ai::AsyncWritable<std::byte> {
public:
  ConnectionWithHeader(std::unique_ptr<ai::AsyncDevice<feature::In<std::byte>>> conn,
                       net::Block block, int32_t size)
      : conn(std::move(conn)), block(std::move(block)), size(size) {}
  ~ConnectionWithHeader() = default;
  coro::Task<Result> write(ai::AsyncDevice<feature::Out<std::byte>>& awd) {
    if (size != block.valid_size) {
      auto [sz, err]
          = co_await awd.write(std::span{block.data.get() + size, block.valid_size - size});
      if (err) {
        co_return {sz, err};
      }
      INFO("Write {} bytes to the device", sz);
    }
    auto [len, err] = co_await io::splice(std::move(conn), &awd, std::string(4096, '\0'));
    co_return {len + block.valid_size - size, err};
  }

private:
  std::unique_ptr<ai::AsyncDevice<feature::In<std::byte>>> conn;
  net::Block block;
  std::size_t size;
};

class Nconn : public Inbound {
public:
  using server_type = tcp::Server<Ip<4>>;
  static std::expected<std::unique_ptr<Nconn>, std::error_condition> make_unique(
      const std::shared_ptr<sync::Poller>& poller, std::string host, std::string port) {
    auto server = server_type::create(host.c_str(), port.c_str(), poller);
    if (!server) {
      return std::unexpected(server.error());
    }
    return std::make_unique<Nconn>(std::move(host), std::move(port), std::move(*server));
  }
  Nconn(std::string host, std::string port, server_type server)
      : Inbound(std::move(host), std::move(port)), server(std::move(server)) {}
  ~Nconn() = default;
  coro::Task<std::tuple<std::size_t, std::optional<std::errc>>> read(
      std::span<InConnection> buf) override {
    auto arwd = co_await server.accept(nullptr);
    if (!arwd) {
      co_return {0, arwd.error()};
    }

    auto [r, w] = std::move(*arwd).split();

    auto buffer = std::make_unique<std::byte[]>(1024);

    auto [sz, err] = co_await r.read(std::span{buffer.get(), 1024});

    if (err) {
      co_return {0, err};
    }
    NconnHeader header;
    auto len = header.deserialize(std::span{buffer.get(), sz});
    INFO("New connection request to {}:{}", header.host, header.port);

    auto conn = std::make_unique<ConnectionWithHeader>(std::move(r).to_unique_dyn(),
                                                       net::Block{std::move(buffer), sz}, len);

    buf[0] = InConnection{.host = std::move(header.host),
                          .port = std::move(header.port),
                          .in = std::move(conn),
                          .out = std::move(w).to_unique_dyn()};

    co_return {1, std::nullopt};
  }

private:
  server_type server;
};

#endif
