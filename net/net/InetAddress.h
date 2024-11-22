#pragma once

#include "net/SocketOps.h"

#include "netinet/in.h"

#include <string>

namespace bamboo {

// the wrapper of sockaddr_in, sockaddr_in6
class InetAddress {
public:
  explicit InetAddress(uint16_t port = 0, std::string ip = "127.0.0.1");

  explicit InetAddress(const sockaddr_in &addr) : addr_(addr) {}

  explicit InetAddress(const sockaddr_in6 &addr) : addr6_(addr) {}

  sa_family_t family() const { return addr_.sin_family; }

  const struct sockaddr *getSockAddr() const { return sockets::sockaddr_cast(&addr6_); }

  std::string toIp() const;

  std::string toIpPort() const;

  uint16_t toPort() const;

  const sockaddr_in *getSockAddrInet() const { return &addr_; }

  void setSockAddrInet(const sockaddr_in &addr) { addr_ = addr; }

private:
  union {
    struct sockaddr_in addr_;
    struct sockaddr_in6 addr6_;
  };
};
} // namespace bamboo