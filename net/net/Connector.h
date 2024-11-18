#pragma once

#include "base/Macro.h"
#include "net/InetAddress.h"

#include <functional>
#include <memory>

namespace bamboo {

class Channel;
class EventLoop;

class Connector : public std::enable_shared_from_this<Connector> {
public:
  using NewConnectionCallback = std::function<void(int sockfd)>;

  Connector(EventLoop *loop, const InetAddress &server_addr);

  DISALLOW_COPY(Connector);

  ~Connector();

  void setNewConnectionCallback(const NewConnectionCallback &cb) {
    new_connection_callback_ = cb;
  }

  void start();
  void restart();
  void stop();

  const InetAddress &serverAddress() const { return server_addr_; }

private:
  enum States { kDisconnected, kConnecting, kConnected };
  static constexpr int kMaxRetryDelayMs = 30 * 1000;
  static constexpr int kInitRetryDelayMs = 500;

  void setState(States s) { state_ = s; }

  void startInLoop();

  void stopInLoop();

  void connect();

  void connecting(int sockfd);

  void handleWrite();

  void handleError();

  void retry(int sockfd);

  int removeAndResetChannel();

  void resetChannel();

  bool connected_;
  int retry_delay_ms_;
  States state_;
  EventLoop *loop_;
  InetAddress server_addr_;
  std::unique_ptr<Channel> channel_;
  NewConnectionCallback new_connection_callback_;
};

} // namespace bamboo