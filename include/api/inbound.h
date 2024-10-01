#ifndef XPROXY_API_INBOUND
#define XPROXY_API_INBOUND
#include <xsl/ai.h>
#include <xsl/feature.h>
#include <xsl/net.h>

#include <cstddef>
#include <memory>
#include <string>
#include <utility>
using namespace xsl;

struct InConnection {
  std::string host, port;
  std::unique_ptr<ai::AsyncWritable<std::byte>> in;
  std::unique_ptr<ai::AsyncDevice<feature::Out<std::byte>>> out;
};

class Inbound : public ai::AsyncDevice<feature::In<InConnection>> {
public:
  Inbound(std::string host, std::string port) : host(std::move(host)), port(std::move(port)) {}
  virtual ~Inbound() = default;
  virtual coro::Task<std::tuple<std::size_t, std::optional<std::errc>>> read(
      std::span<InConnection> buf) override
      = 0;

protected:
  std::string host, port;
};
#endif
