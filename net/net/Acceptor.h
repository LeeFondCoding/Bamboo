#pragma once

#include "net/Channel.h"
#include "net/Socket.h"

#include <functional>

namespace bamboo {

class EventLoop;

// wrapper of listening socket
class Acceptor {
public:
  using NewConnectionCallback = std::function<void(int sockfd, const InetAddress&)>;

  Acceptor(EventLoop *loop, const InetAddress &listenAddr, bool reuseport);

  ~Acceptor();
  void setNewConnectionCallback(NewConnectionCallback cb) {
    new_connection_callback_ = cb;
  }

  bool listening() const { return listenning_; }

  void listen();

private:
  // handle new connection
  void handleRead();

  EventLoop *loop_;

  Socket accept_socket_;
  Channel accept_channel_;

  NewConnectionCallback new_connection_callback_;

  bool listenning_{false};
};
} // namespace bamboo