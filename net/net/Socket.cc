#include "net/Socket.h"

#include "base/Logging.h"
#include "net/InetAddress.h"
#include "net/SocketOps.h"

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
  sockets::bindOrDie(fd(), sockets::sockaddr_cast(localaddr.getSockAddrInet()));
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

void Socket::setReusePort(bool on) {
#ifdef SO_REUSEPORT
  int optval = on ? 1 : 0;
  int ret = ::setsockopt(fd(), SOL_SOCKET, SO_REUSEPORT, &optval,
                         static_cast<socklen_t>(sizeof optval));
  if (ret < 0 && on) {
    LOG_SYSERR << "SO_REUSEPORT failed.";
  }
#else
  if (on) {
    LOG_ERROR << "SO_REUSEPORT is not supported.";
  }
#endif
}

void Socket::setKeepAlive(bool on) {
  int optval = on ? 1 : 0;
  ::setsockopt(fd_, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof optval);
}

} // namespace bamboo