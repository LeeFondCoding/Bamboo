#include "net/Channel.h"

#include "base/Logging.h"
#include "net/EventLoop.h"

#include <sys/epoll.h>

#include <sstream>

namespace bamboo {

Channel::Channel(EventLoop *loop, int fd) : loop_(loop), fd_(fd) {}

// ???
void Channel::handleEvent(TimeStamp receiveTime) {
  if (tied_) {
    auto guard = tie_.lock();
    if (guard) {
      handleEventWithGuard(receiveTime);
    }
  } else {
    handleEventWithGuard(receiveTime);
  }
}

void Channel::tie(const std::shared_ptr<void> &obj) {
  tie_ = obj;
  tied_ = true;
}

void Channel::remove() { loop_->removeChannel(this); }

void Channel::update() { loop_->updateChannel(this); }

void Channel::handleEventWithGuard(TimeStamp receiveTime) {
  LOG_INFO << "channel handleevent revents : " << reventsToString();
  if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN)) {
    LOG_WARN << "fd = " << fd_ << " HUP";
    if (close_callback_) {
      close_callback_();
    }
  }
  if (revents_ & EPOLLERR) {
    LOG_ERROR << "fd = " << fd_ << " error";
    if (err_callback_) {
      err_callback_();
    }
  }
  if (revents_ & (EPOLLIN | EPOLLPRI)) {
    if (read_callback_) {
      read_callback_(receiveTime);
    }
  }
  if (revents_ & EPOLLOUT) {
    if (write_callback_) {
      write_callback_();
    }
  }
}

std::string Channel::reventsToString() const {
  return eventsToString(revents_);
}

std::string Channel::eventsToString() const { return eventsToString(events_); }

std::string Channel::eventsToString(int events) const {
  std::ostringstream oss;
  oss << fd_ << ": ";
  if (events & EPOLLIN) {
    oss << "IN ";
  }
  if (events & EPOLLOUT) {
    oss << "OUT ";
  }
  if (events & EPOLLPRI) {
    oss << "PRI ";
  }
  if (events & EPOLLERR) {
    oss << "ERR ";
  }
  if (events & EPOLLHUP) {
    oss << "HUP ";
  }
  if (events & EPOLLRDHUP) {
    oss << "RDHUP ";
  }
  if (events & EPOLLONESHOT) {
    oss << "ONESHOT ";
  }
  return oss.str();
}

} // namespace bamboo