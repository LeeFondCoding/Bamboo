#include "net/BambooServer.h"

#include "base/Logging.h"
#include "control/ClientSession.h"
#include "control/DatabaseManager.h"
#include "net/TcpConnection.h"

namespace bamboo {

BambooServer::BambooServer(EventLoop *loop, const InetAddress &listenAddr)
    : server_(loop, listenAddr, "KVServer"),
      db_manager_(new DatabaseManager()) {
  server_.setConnectionCallback(
      std::bind(&BambooServer::onConnection, this, std::placeholders::_1));
  server_.setMessageCallback(
      std::bind(&BambooServer::onMessage, this, std::placeholders::_1,
                std::placeholders::_2, std::placeholders::_3));
}

 BambooServer::~BambooServer() = default;

void BambooServer::onConnection(const TcpConnectionPtr &conn) {
  LOG_INFO << "KVServer - " << conn->peerAddress().toIpPort() << " -> "
           << conn->localAddress().toIpPort() << " is "
           << (conn->connected() ? "UP" : "DOWN");

  if (conn->connected()) {
    sessions_[conn] = std::make_shared<ClientSession>(db_manager_.get());
  } else {
    sessions_.erase(conn);
  }
}

void BambooServer::onMessage(const TcpConnectionPtr &conn, Buffer *buf,
                             TimeStamp time) {
  auto session = sessions_.find(conn);
  if (session == sessions_.end()) {
    conn->send("ERROR: No session found\r\n");
    return;
  }

  std::string msg(buf->retrieveAllString());
  size_t pos = msg.find('\n');
  if (pos == std::string::npos) {
    return;
  }
  std::string command = msg.substr(0, pos);

  size_t spacePos = command.find(' ');
  std::string cmd;
  std::string args;
  if (spacePos != std::string::npos) {
    cmd = command.substr(0, spacePos);
    args = command.substr(spacePos + 1);
  } else {
    cmd = command;
  }

  std::string response = session->second->processCommand(cmd, args);
  conn->send(response);
}
} // namespace bamboo