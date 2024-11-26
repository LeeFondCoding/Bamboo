#pragma once

#include "base/Macro.h"
#include "net/Buffer.h"
#include "net/CallBack.h"

#include <atomic>
#include <unordered_map>

namespace bamboo {

class Acceptor;
class EventLoop;
class EventLoopThreadPool;
class InetAddress;

class TcpServer {
public:
  using ThreadInitCallback = std::function<void(EventLoop *)>;

  enum Option { kNoReusePort, kReusePort };

  TcpServer(EventLoop *loop, const InetAddress &listenAddr,
            const std::string &nameArg, Option option = Option::kNoReusePort);

  DISALLOW_COPY(TcpServer);

  ~TcpServer();

  void setThreadInitCallback(const ThreadInitCallback &cb) {
    thread_init_callback_ = cb;
  }

  void setConnectionCallback(const ConnectionCallback &cb) {
    connection_callback_ = cb;
  }

  void setMessageCallback(const MessageCallback &cb) { message_callback_ = cb; }

  void setWriteCompleteCallback(const WriteCompleteCallback &cb) {
    write_complete_callback_ = cb;
  }

  void setThreadNum(int threads_num);

  void start();

  const std::string &name() const { return name_; }

  const std::string &ipPort() const { return ip_port_; }

private:
  void newConnection(int sockfd, const InetAddress &peerAddr);
  void removeConnection(const TcpConnectionPtr &conn);
  void removeConnnectionInLoop(const TcpConnectionPtr &conn);

  using ConnectionMap = std::unordered_map<std::string, TcpConnectionPtr>;

  EventLoop *loop_;

  const std::string ip_port_;
  const std::string name_;

  std::unique_ptr<Acceptor> acceptor_;
  std::shared_ptr<EventLoopThreadPool> thread_pool_;

  ConnectionCallback connection_callback_;
  MessageCallback message_callback_;
  WriteCompleteCallback write_complete_callback_;
  ThreadInitCallback thread_init_callback_;

  std::atomic<int> started_{0};
  int next_conn_id_{1};
  ConnectionMap connections_;
};
} // namespace bamboo