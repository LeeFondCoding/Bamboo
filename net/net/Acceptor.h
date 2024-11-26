#pragma once

#include "base/Macro.h"
#include "net/Channel.h"
#include "net/Socket.h"

#include <functional>

namespace bamboo {

class EventLoop;

// wrapper of listening socket
// handle new connection
class Acceptor {
public:
  using NewConnectionCallback = std::function<void(int sockfd, const InetAddress&)>;

  Acceptor(EventLoop *loop, const InetAddress &listenAddr, bool reuseport);

  DISALLOW_COPY(Acceptor)

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
  int idle_fd_;
};
} // namespace bamboo