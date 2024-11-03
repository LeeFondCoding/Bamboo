#pragma once

#include "base/Macro.h"

namespace bamboo {

class InetAddress;

// wrapper of socket fd
class Socket {
public:
  explicit Socket(int fd) : fd_(fd) {}

  ~Socket();

  DISALLOW_COPY(Socket);

  //return socket fd
  int fd() const { return fd_; }

 // for server bind address
  void bindAddress(const InetAddress &localAddr);

  // listen for new connections
  // socket for main loop
  void listen();

  // return non-block and close-exec socket
  // return -1 if errors happen
  int accept(InetAddress *peerAddr);

  // elegant way to close write side of socket
  void shutdownWrite();

  // turn on Nagle algorithm
  void setTcpNoDelay(bool on);

  void setReuseAddr(bool on);

  void setReusePort(bool on);

  // keep tcp connection alive
  void setKeepAlive(bool on);

private:
  // socket fd
  const int fd_;
};
} // namespace bamboo