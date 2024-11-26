#pragma once

#include "base/Macro.h"
#include "base/TimeStamp.h"

#include <functional>
#include <memory>

namespace bamboo {

class EventLoop;

// handle events on fd(socket, timefd, eventfd)
// run callback function according to its events
class Channel {
public:
  using EventCallback = std::function<void()>;
  using ReadEventCallback = std::function<void(TimeStamp)>;

  Channel(EventLoop *loop, int fd);

  DISALLOW_COPY(Channel);

  ~Channel();

  void handleEvent(TimeStamp receiveTime);

  void setReadCallback(ReadEventCallback cb) { read_callback_ = std::move(cb); }

  void setWriteCallback(EventCallback cb) { write_callback_ = std::move(cb); }

  void setCloseCallback(EventCallback cb) { close_callback_ = std::move(cb); }

  void setErrorCallback(EventCallback cb) { err_callback_ = std::move(cb); }

  // tie this channle to according TcpConnection
  void tie(const std::shared_ptr<void> &);

  int fd() const { return fd_; }

  int events() const { return events_; }

  void setRevents(int revents) { revents_ = revents; }

  void enableReading() {
    events_ |= kReadEvent;
    update();
  }

  void disableReading() {
    events_ &= ~kReadEvent;
    update();
  }

  void enableWriting() {
    events_ |= kWriteEvent;
    update();
  }

  void disableWriting() {
    events_ &= ~kWriteEvent;
    update();
  }

  void disableAll() {
    events_ = kNoneEvent;
    update();
  }

  bool isWriting() const { return events_ & kWriteEvent; }

  bool isReading() const { return events_ & kReadEvent; }

  bool isNoneEvent() const { return events_ == kNoneEvent; }

  int index() { return index_; }

  void setIndex(int idx) { index_ = idx; }

  EventLoop *ownerLoop() { return loop_; }

  void remove();

  // for debug
  std::string reventsToString() const;
  std::string eventsToString() const;

private:
  std::string eventsToString(int events) const;

  void update();

  void handleEventWithGuard(TimeStamp receiveTime);

  static const int kNoneEvent;
  static const int kReadEvent;
  static const int kWriteEvent;


  const int fd_;
  int events_{0};
  int revents_{0};
  int index_{-1};
  bool tied_{false};
  bool event_handing_{false};
  bool added_to_loop_{false};
  bool log_hup_{true};

  EventLoop *loop_;
  std::weak_ptr<void> tie_;

  ReadEventCallback read_callback_;
  EventCallback write_callback_;
  EventCallback err_callback_;
  EventCallback close_callback_;
};

} // namespace bamboo