#pragma once

#include "net/TcpClient.h"

namespace bamboo {

class BambooClient {
public:
  BambooClient(EventLoop *loop, const InetAddress &serverAddr);

  void connect() { client_.connect(); }

private:
  void onConnection(const TcpConnectionPtr &conn);

  void onMessage(const TcpConnectionPtr &conn, Buffer *buf, TimeStamp time);

  void promptUser(const TcpConnectionPtr &conn);

  TcpClient client_;
};

} // namespace bamboo