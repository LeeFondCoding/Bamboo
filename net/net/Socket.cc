#include "net/Socket.h"

#include "base/Logging.h"
#include "net/InetAddress.h"

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>

namespace bamboo {
Socket::~Socket() {
  if (::close(fd_) < 0) {
    LOG_SYSFATAL << "close socket: " << fd_ << " failed";
  }
}

void Socket::bindAddress(const InetAddress &localaddr) {
  auto ret =
      bind(fd_, (sockaddr *)localaddr.getSockAddrInet(), sizeof(sockaddr_in));
  if (ret < 0) {
    LOG_SYSFATAL << "bind socket: " << fd_ << " failed";
  }
}

void Socket::listen() {
  if (::listen(fd_, SOMAXCONN) != 0) {
    LOG_SYSFATAL << "listen socket: " << fd_ << " failed";
  }
}

int Socket::accept(InetAddress *peer_addr) {
  sockaddr_in addr;
  socklen_t addr_len = sizeof addr;
  bzero(&addr, sizeof(addr));
  auto connfd = ::accept4(fd_, (sockaddr *)&addr, &addr_len,
                          SOCK_NONBLOCK | SOCK_CLOEXEC);
  if (connfd >= 0) {
    peer_addr->setSockAddrInet(addr);
  }
  return connfd;
}

void Socket::shutdownWrite() {
  if (::shutdown(fd_, SHUT_WR) < 0) {
    LOG_SYSERR << "shutdownWrite";
  }
}

void Socket::setTcpNoDelay(bool on) {
  int optval = on ? 1 : 0;
  ::setsockopt(fd_, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof optval);
}

void Socket::setReuseAddr(bool on) {
  int optval = on ? 1 : 0;
  ::setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
}

void Socket::setKeepAlive(bool on) {
  int optval = on ? 1 : 0;
  ::setsockopt(fd_, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof optval);
}

} // namespace bamboo