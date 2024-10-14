#pragma once

#include "base/Macro.h"

namespace bamboo {

class InetAddress;

class Socket {
public:
  explicit Socket(int fd) : fd_(fd) {}

  ~Socket();

  DISALLOW_COPY(Socket);

  int fd() const { return fd_; }

  void bindAddress(const InetAddress &localAddr);

  void listen();

  int accept(InetAddress *peerAddr);

  void shutdownWrite();

  void setTcpNoDelay(bool on);

  void setReuseAddr(bool on);

  void setReusePort(bool on);

  void setKeepAlive(bool on);

private:
  const int fd_;
};
} // namespace bamboo