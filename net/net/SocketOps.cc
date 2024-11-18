#include "net/SocketOps.h"

#include "base/Logging.h"


#include <assert.h>
#include <errno.h>
#include <unistd.h>
#include <sys/uio.h>

namespace bamboo {
namespace sockets {

int createNonBlockingSocketFd(sa_family_t family) {
  int sockfd =
      ::socket(family, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
  if (sockfd < 0) {
    LOG_SYSFATAL << "sockets::createNonblockingOrDie";
  }
  return sockfd;
}

int connect(int sockfd, const struct sockaddr *addr) {
  return ::connect(sockfd, addr,
                   static_cast<socklen_t>(sizeof(struct sockaddr_in6)));
}

void bindOrDie(int sockfd, const struct sockaddr *addr) {
  int ret =
      ::bind(sockfd, addr, static_cast<socklen_t>(sizeof(struct sockaddr_in6)));
  if (ret < 0) {
    LOG_SYSFATAL << "sockets::bindOrDie";
  }
}

void listenOrDie(int sockfd) {
  int ret = ::listen(sockfd, SOMAXCONN);
  if (ret < 0) {
    LOG_SYSFATAL << "sockets::listenOrDie";
  }
}

int accept(int sockfd, struct sockaddr_in6 *addr) {
  socklen_t addrlen = static_cast<socklen_t>(sizeof *addr);
  int connfd = ::accept4(sockfd, sockaddr_cast(addr), &addrlen,
                         SOCK_NONBLOCK | SOCK_CLOEXEC);

  if (connfd < 0) {
    int saved_err = errno;
    LOG_SYSERR << "Socket::accept";
    switch (saved_err) {
    case EAGAIN:
    case ECONNABORTED:
    case EINTR:
    case EPROTO: // ???
    case EPERM:
    case EMFILE: // per-process lmit of open file desctiptor ???
      // expected errors
      errno = saved_err;
      break;
    case EBADF:
    case EFAULT:
    case EINVAL:
    case ENFILE:
    case ENOBUFS:
    case ENOMEM:
    case ENOTSOCK:
    case EOPNOTSUPP:
      // unexpected errors
      LOG_FATAL << "unexpected error of ::accept " << saved_err;
      break;
    default:
      LOG_FATAL << "unknown error of ::accept " << saved_err;
      break;
    }
  }
  return connfd;
}

ssize_t read(int sockfd, void *buf, size_t count) {
  return ::read(sockfd, buf, count);
}

ssize_t readv(int sockfd, const struct iovec *iov, int iovcnt) {
  return ::readv(sockfd, iov, iovcnt);
}

ssize_t write(int sockfd, const void *buf, size_t count) {
  return ::write(sockfd, buf, count);
}

void close(int sockfd) {
  if (::close(sockfd) < 0) {
    LOG_SYSERR << "sockets::close";
  }
}

void shutdownWrite(int sockfd) {
  if (::shutdown(sockfd, SHUT_WR) < 0) {
    LOG_SYSERR << "sockets::shutdownWrite";
  }
}

int getSocketError(int sockfd) {
  int optval;
  socklen_t optlen = static_cast<socklen_t>(sizeof optval);

  if (::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0) {
    return errno;
  } else {
    return optval;
  }
}

const struct sockaddr *sockaddr_cast(const struct sockaddr_in6 *addr) {
  return reinterpret_cast<const struct sockaddr *>(addr);
}

struct sockaddr *sockaddr_cast(struct sockaddr_in6 *addr) {
  return reinterpret_cast<struct sockaddr *>(addr);
}

const struct sockaddr *sockaddr_cast(const struct sockaddr_in *addr) {
  return reinterpret_cast<const struct sockaddr *>(addr);
}

const struct sockaddr_in *sockaddr_in_cast(const struct sockaddr *addr) {
  return reinterpret_cast<const struct sockaddr_in *>(addr);
}

const struct sockaddr_in6 *sockaddr_in6_cast(const struct sockaddr *addr) {
  return reinterpret_cast<const struct sockaddr_in6 *>(addr);
}

struct sockaddr_in6 getLocalAddr(int sockfd) {
  struct sockaddr_in6 localaddr;
  bzero(&localaddr, sizeof localaddr);
  socklen_t addrlen = static_cast<socklen_t>(sizeof localaddr);
  if (::getsockname(sockfd, sockaddr_cast(&localaddr), &addrlen) < 0) {
    LOG_SYSERR << "sockets::getLocalAddr";
  }
  return localaddr;
}

struct sockaddr_in6 getPeerAddr(int sockfd) {
  struct sockaddr_in6 peeraddr;
  bzero(&peeraddr, sizeof peeraddr);
  socklen_t addrlen = static_cast<socklen_t>(sizeof peeraddr);
  if (::getpeername(sockfd, sockaddr_cast(&peeraddr), &addrlen) < 0) {
    LOG_SYSERR << "sockets::getPeerAddr";
  }
  return peeraddr;
}

bool isSelfConnect(int sockfd) {
  struct sockaddr_in6 localaddr = getLocalAddr(sockfd);
  struct sockaddr_in6 peeraddr = getPeerAddr(sockfd);
  if (localaddr.sin6_family == AF_INET) {
    const struct sockaddr_in *laddr4 =
        reinterpret_cast<struct sockaddr_in *>(&localaddr);
    const struct sockaddr_in *raddr4 =
        reinterpret_cast<struct sockaddr_in *>(&peeraddr);
    return laddr4->sin_port == raddr4->sin_port &&
           laddr4->sin_addr.s_addr == raddr4->sin_addr.s_addr;
  } else if (localaddr.sin6_family == AF_INET6) {
    return localaddr.sin6_port == peeraddr.sin6_port &&
           memcmp(&localaddr.sin6_addr, &peeraddr.sin6_addr,
                  sizeof localaddr.sin6_addr) == 0;
  } else {
    return false;
  }
}

} // namespace sockets
} // namespace bamboo