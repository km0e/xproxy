#pragma once
#ifndef XPROXY_API_PROTO_NCONN
#  define XPROXY_API_PROTO_NCONN
#  include <xsl/wheel.h>

#  include <format>
#  include <span>
#  include <string>
class NconnHeader {
public:
  NconnHeader() : host(), port() {}
  NconnHeader(std::string host, std::string port) : host(std::move(host)), port(std::move(port)) {}
  ~NconnHeader() = default;
  std::string serialize() const {
    auto header = std::format("    {}:{}", host, port);
    xsl::i32_to_bytes(header.size(), std::as_writable_bytes(std::span{header}));
    return header;
  }
  int32_t deserialize(std::span<const std::byte> buf) {
    auto size = xsl::i32_from_bytes(buf);
    buf = buf.subspan(4);
    auto header = std::string(reinterpret_cast<const char*>(buf.data()), size - 4);
    auto pos = header.find(':');
    host = header.substr(0, pos);
    port = header.substr(pos + 1);
    return size;
  }

  std::string host;
  std::string port;
};

#endif
