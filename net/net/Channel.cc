#include "net/Channel.h"

#include "net/EventLoop.h"

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

  void Channel::tie(const std::shared_ptr<void> & obj) {
    tie_ = obj;
    tied_ = true;
  }

  void Channel::remove() {
    loop_->removeChannel(this);
  }

  void Channel::update() {
    loop_->updateChannel(this);
  }

  void Channel::handleEventWithGuard(TimeStamp receiveTime) {
    LOG_INFO 
  }

} // namespace bamboo