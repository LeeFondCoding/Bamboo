#pragma once

#include "net/TcpServer.h"

namespace bamboo {

class ClientSession;
class DatabaseManager;

class BambooServer {
public:
  BambooServer(EventLoop *loop, const InetAddress &listenAddr);

  ~BambooServer();

  void start() { server_.start(); }

private:
  void onConnection(const TcpConnectionPtr &conn);

  void onMessage(const TcpConnectionPtr &conn, Buffer *buf, TimeStamp time);

  TcpServer server_;
  std::unique_ptr<DatabaseManager> db_manager_;
  std::unordered_map<TcpConnectionPtr, std::shared_ptr<ClientSession>>
      sessions_;
};

} // namespace bamboo