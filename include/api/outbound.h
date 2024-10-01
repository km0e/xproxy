#ifndef XPROXY_API_OUTBOUND
#define XPROXY_API_OUTBOUND
#include <xsl/ai.h>
#include <xsl/feature.h>
#include <xsl/net.h>

#include <cstddef>
#include <memory>
#include <string>
#include <utility>
using namespace xsl;

struct OutConnection {
  std::string ip, port;
  std::unique_ptr<ai::AsyncDevice<feature::In<std::byte>>> in;
  std::unique_ptr<ai::AsyncDevice<feature::Out<std::byte>>> out;
};

class Outbound : public ai::AsyncDevice<feature::In<OutConnection>> {
public:
  Outbound(std::string host, std::string port) : host(std::move(host)), port(std::move(port)) {}
  virtual ~Outbound() = default;
  virtual coro::Task<std::tuple<std::size_t, std::optional<std::errc>>> read(
      std::span<OutConnection> buf) override
      = 0;

protected:
  std::string host, port;
};
#endif
