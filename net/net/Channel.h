#pragma once

#include "base/Macro.h"
#include "base/TimeStamp.h"

#include <functional>
#include <memory>

namespace bamboo {

class EventLoop;

class Channel {
public:
  using EventCallback = std::function<void()>;
  using ReadEventCallback = std::function<void(TimeStamp)>;

  Channel(EventLoop *loop, int fd);

  ~Channel() = default;

  DISALLOW_COPY(Channel);

  void handleEvent(TimeStamp receiveTime);

  void setReadCallback(ReadEventCallback cb) { readCallback_ = std::move(cb); }

  void setWriteCallback(EventCallback cb) { writeCallback_ = std::move(cb); }

  void setCloseCallback(EventCallback cb) { closeCallback_ = std::move(cb); }

  void setErrorCallback(EventCallback cb) { errorCallback_ = std::move(cb); }

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
  
  void set_index(int idx) { index_ = idx; }

  EventLoop *ownerLoop() { return loop_; }

  void remove();

private:
  void update();

  void handleEventWithGuard(TimeStamp receiveTime);

  static const int kNoneEvent;
  static const int kReadEvent;
  static const int kWriteEvent;
  EventLoop *loop_;
  const int fd_;
  int events_{0};
  int revents_{0};
  int index_{-1};
  std::weak_ptr<void> tie_;
  bool tied_{false};
  bool event_handing_;
  bool added_to_loop_;

  ReadEventCallback readCallback_;
  EventCallback writeCallback_;
  EventCallback errorCallback_;
  EventCallback closeCallback_;
};

} // namespace bamboo