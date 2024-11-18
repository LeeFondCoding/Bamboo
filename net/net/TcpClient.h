#pragma once

#include "base/Macro.h"
#include "net/TcpConnection.h"

#include <mutex>

namespace bamboo {

class Connector;
using ConnectorPtr = std::shared_ptr<Connector>;

class TcpClient {
public:
  TcpClient(EventLoop *loop, const InetAddress &serverAddr,
            const std::string &nameArg);

  DISALLOW_COPY(TcpClient);
  
  ~TcpClient();
  void connect();
  void disconnect();
  void stop();

  TcpConnectionPtr connection() const {
    std::lock_guard<std::mutex> lck{mutex_};
    return connection_;
  }

  EventLoop *getLoop() const { return loop_; }

  bool retry() const { return retry_; }

  void enableRetry() { retry_ = true; }

  const std::string &name() const { return name_; }

  void setConnectionCallback(const ConnectionCallback &cb) {
    connectionCallback_ = cb;
  }

  void setMessageCallback(const MessageCallback &cb) { messageCallback_ = cb; }

  void setWriteCompleteCallback(const WriteCompleteCallback &cb) {
    writeCompleteCallback_ = cb;
  }

private:
  void newConnection(int sockfd);

  void removeConnection(const TcpConnectionPtr &conn);

  EventLoop *loop_;
  ConnectorPtr connector_;
  const std::string name_;
  bool retry_{false};
  bool connect_{false};
  int next_conn_id_{1};
  mutable std::mutex mutex_;
  TcpConnectionPtr connection_; // guarded by mutex_

  ConnectionCallback connectionCallback_;
  MessageCallback messageCallback_;
  WriteCompleteCallback writeCompleteCallback_;
};

} // namespace bamboo