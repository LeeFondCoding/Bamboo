#include "net/Channel.h"

#include "base/Logging.h"
#include "net/EventLoop.h"

#include <assert.h>
#include <poll.h>

#include <sstream>

namespace bamboo {

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventLoop *loop, int fd) : loop_(loop), fd_(fd) {}

Channel::~Channel() {
  assert(!event_handing_);
  assert(!added_to_loop_);
  if (loop_->isInLoopThread()) {
    assert(!loop_->hasChannel(this));
  }
}

// ???
void Channel::handleEvent(TimeStamp receive_time) {
  if (tied_) {
    auto guard = tie_.lock();
    if (guard) {
      handleEventWithGuard(receive_time);
    }
  } else {
    handleEventWithGuard(receive_time);
  }
}

void Channel::tie(const std::shared_ptr<void> &obj) {
  tie_ = obj;
  tied_ = true;
}

void Channel::remove() {
  assert(isNoneEvent());
  added_to_loop_ = false;
  loop_->removeChannel(this);
}

void Channel::update() {
  added_to_loop_ = true;
  loop_->updateChannel(this);
}

void Channel::handleEventWithGuard(TimeStamp receiveTime) {
  event_handing_ = true;
  LOG_TRACE << reventsToString();
  if ((revents_ & POLLHUP) && !(revents_ & POLLIN)) {
    if (log_hup_) {
      LOG_WARN << "fd = " << fd_ << " Channel::handle_event() POLLHUP";
    }
    if (close_callback_) {
      close_callback_();
    }
  }

  if (revents_ & POLLNVAL) {
    LOG_WARN << "fd = " << fd_ << " Channel::handle_event() POLLNVAL";
  }

  if (revents_ & (POLLERR | POLLNVAL)) {
    if (err_callback_)
      err_callback_();
  }
  if (revents_ & (POLLIN | POLLPRI | POLLRDHUP)) {
    if (read_callback_)
      read_callback_(receiveTime);
  }
  if (revents_ & POLLOUT) {
    if (write_callback_)
      write_callback_();
  }
  event_handing_ = false;
}

std::string Channel::reventsToString() const {
  return eventsToString(revents_);
}

std::string Channel::eventsToString() const { return eventsToString(events_); }

std::string Channel::eventsToString(int events) const {
  std::ostringstream oss;
  oss << fd_ << ": ";
  if (events & POLLIN)
    oss << "IN ";
  if (events & POLLPRI)
    oss << "PRI ";
  if (events & POLLOUT)
    oss << "OUT ";
  if (events & POLLHUP)
    oss << "HUP ";
  if (events & POLLRDHUP)
    oss << "RDHUP ";
  if (events & POLLERR)
    oss << "ERR ";
  if (events & POLLNVAL)
    oss << "NVAL ";

  return oss.str();
}

} // namespace bamboo