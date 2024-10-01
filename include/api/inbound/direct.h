#pragma once
#ifndef XPROXY_API_INBOUND_DIRECT
#  define XPROXY_API_INBOUND_DIRECT
// class DirectConfig {
// public:
//   std::string address;
//   int port;
// };
// class DirectConnection : public ai::AsyncDevice<feature::InOut<std::byte>> {
// public:
//   DirectConnection(DirectConfig config) : config(std::move(config)) {}
//   coro::Task<std::tuple<std::size_t, std::optional<std::errc>>> read(
//       std::span<std::byte> buf) override {}
//   coro::Task<std::tuple<std::size_t, std::optional<std::errc>>> write(
//       std::span<const std::byte> buf) override {}

// private:
//   DirectConfig config;
// };
// class Direct {
// public:
//   template <typename T>
//   Direct(T&& config) : config(std::forward<T>(config)) {}
//   coro::Task<std::tuple<std::size_t, std::optional<std::errc>>> read(
//       std::span<std::byte> buf) override {}
//   coro::Task<std::tuple<std::size_t, std::optional<std::errc>>> write(
//       std::span<const std::byte> buf) override {}

// private:
//   DirectConfig config;
// };
#endif
