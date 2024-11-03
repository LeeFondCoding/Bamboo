#pragma once

#include "base/Macro.h"
#include "net/Buffer.h"
#include "net/CallBack.h"
#include "net/InetAddress.h"

#include <atomic>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

namespace bamboo {

class Channel;
class EventLoop;
class HttpContext;
class InetAddress;
class Socket;

class TcpConnection : public std::enable_shared_from_this<TcpConnection> {
public:
  TcpConnection(EventLoop *loop, const std::string &name, int sockfd,
                const InetAddress &localAddr, const InetAddress &peerAddr);

  ~TcpConnection();

  DISALLOW_COPY(TcpConnection)

  EventLoop *getLoop() const { return loop_; }
  const InetAddress &localAddress() const { return local_addr_; }
  const InetAddress &peerAddress() const { return peer_addr_; }
  bool connected() const { return state_ == StateE::kConnected; }
  void send(const std::string &buf);
  void send(Buffer *buf);
  void shutdown();
  const std::string &name() const { return name_; }
  void setConnectionCallback(const ConnectionCallback &cb) {
    connection_call_back_ = cb;
  }
  void setMessageCallback(const MessageCallback &cb) {
    message_call_back_ = cb;
  }
  void setWriteCompleteCallback(const WriteCompleteCallback &cb) {
    write_complete_call_back_ = cb;
  }
  void setCloseCallback(const CloseCallback &cb) { close_callback_ = cb; }
  void setHighWaterMarkCallback(const HighWaterMarkCallback &cb,
                                size_t highWaterMark) {
    high_water_mark_call_back_ = cb;
    highWaterMark = highWaterMark;
  }

  void connectEstablished();

  void connectDestroyed();

  void setContext(HttpContext *context) { context_.reset(context); }

  HttpContext *getMutableContext() { return context_.get(); }

private:
  enum StateE { kDisconnected = 0, kConnecting, kConnected, kDisconnecting };

  void setState(StateE s) { state_ = s; }

  void handleRead(TimeStamp receiveTime);

  void handleWrite();

  void handleClose();

  void handleError();

  void sendInLoop(const std::string &message);

  void sendInLoop(const char *message, size_t len);

  void shutdownInLoop();

  EventLoop *loop_;
  const std::string name_;
  std::atomic<int> state_;
  bool reading_;

  std::unique_ptr<Socket> socket_;
  std::unique_ptr<Channel> channel_;
  std::unique_ptr<HttpContext> context_;

  const InetAddress local_addr_;
  const InetAddress peer_addr_;

  ConnectionCallback connection_call_back_;
  MessageCallback message_call_back_;
  WriteCompleteCallback write_complete_call_back_;
  HighWaterMarkCallback high_water_mark_call_back_;
  size_t high_water_mark_;
  CloseCallback close_callback_;
  Buffer input_buffer_;
  Buffer output_buffer_;

  // 64M
  static constexpr size_t kHighWaterMark = 64 * 1024 * 1024;
};
} // namespace bamboo