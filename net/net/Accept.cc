#include "net/Acceptor.h"

#include "base/Logging.h"
#include "net/InetAddress.h"
#include "net/SocketOps.h"

namespace bamboo {

Acceptor::Acceptor(EventLoop *loop, const InetAddress &listen_addr,
                   bool reuse_port)
    : loop_(loop), accept_socket_(sockets::createNonBlockingSocketFd(listen_addr.family())),
      accept_channel_(loop_, accept_socket_.fd()) {
  accept_socket_.setReuseAddr(true);
  accept_socket_.setReusePort(reuse_port);
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
      sockets::close(conn_fd);
    }
  } else {
    LOG_SYSERR << "Acceptor::handleRead";
    if (errno == EMFILE) {
      LOG_ERROR << "Too many open files";
    }
  }
}

} // namespace bamboo