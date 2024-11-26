#include "net/BambooClient.h"

#include "base/Logging.h"

#include <iostream>

namespace bamboo {

BambooClient::BambooClient(EventLoop *loop, const InetAddress &serverAddr)
    : client_(loop, serverAddr, "BambooClient") {
  client_.setConnectionCallback(
      std::bind(&BambooClient::onConnection, this, std::placeholders::_1));
  client_.setMessageCallback(
      std::bind(&BambooClient::onMessage, this, std::placeholders::_1,
                std::placeholders::_2, std::placeholders::_3));
}

void BambooClient::onConnection(const TcpConnectionPtr &conn) {
  LOG_INFO << "Connected to server at " << conn->peerAddress().toIpPort();

  if (conn->connected()) {
    promptUser(conn);
  } else {
    LOG_INFO << "Disconnected from server";
  }
}

void BambooClient::onMessage(const TcpConnectionPtr &conn, Buffer *buf,
                             TimeStamp time) {
  std::string response(buf->retrieveAllString());
  std::cout << "Received: " << response;
  promptUser(conn);
}

void BambooClient::promptUser(const TcpConnectionPtr &conn) {
  std::cout << "Enter > ";
  std::string input;
  std::getline(std::cin, input);

  if (!input.empty()) {
    conn->send(input + "\n");
  }
}

} // namespace bamboo