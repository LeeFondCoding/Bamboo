#include "net/Acceptor.h"

#include "base/Logging.h"
#include "net/InetAddress.h"

#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

namespace {
static int newNonBlockSocket() {
  int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC,
                        IPPROTO_TCP);
  if (sockfd < 0) {
    bamboo::LOG_SYSFATAL << "Failed in socket";
  }
  return sockfd;
}

} // namespace

namespace bamboo {

Acceptor::Acceptor(EventLoop *loop, const InetAddress &listen_addr,
                   bool reuse_port)
    : loop_(loop), accept_socket_(newNonBlockSocket()),
      accept_channel_(loop_, accept_socket_.fd()) {
  accept_socket_.setReuseAddr(true);
  accept_socket_.setReuseAddr(true);
  accept_socket_.bindAddress(listen_addr);
  accept_channel_.setReadCallback(std::bind(&Acceptor::handleRead, this));
}

Acceptor::~Acceptor() {
  accept_channel_.disableAll();
  accept_channel_.remove();
}

void Acceptor::listen() {
  listenning_ = true;
  accept_socket_.listen();
  accept_channel_.enableReading();
}

void Acceptor::handleRead() {
  InetAddress peer_addr;
  auto conn_fd = accept_socket_.accept(&peer_addr);

  if (conn_fd >= 0) {
    if (new_connection_callback_) {
      new_connection_callback_(conn_fd, peer_addr);
    } else {
      ::close(conn_fd);
    }
  } else {
    LOG_SYSERR << "Acceptor::handleRead";
    if (errno == EMFILE) {
      LOG_ERROR << "Too many open files";
    }
  }
}

} // namespace bamboo