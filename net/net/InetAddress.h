#pragma once

#include "netinet/in.h"

#include <string>

namespace bamboo {

class InetAddress {
public:
  explicit InetAddress(uint16_t port = 0, std::string ip = "127.0.0.1");

  explicit InetAddress(const sockaddr_in &addr) : addr_(addr) {}

  std::string toIp() const;

  std::string toIpPort() const;

  uint16_t toPort() const;
  
  const sockaddr_in *getSockAddrInet() const { return &addr_; }

  void setSockAddrInet(const sockaddr_in &addr) { addr_ = addr; }

private:
  struct sockaddr_in addr_;
};
} // namespace bamboo