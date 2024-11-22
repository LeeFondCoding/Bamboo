#include "net/Poller.h"

#include "net/Channel.h"
#include "net/EpollPoller.h"
#include "net/EventLoop.h"

namespace bamboo {

Poller::Poller(EventLoop *loop) : loop_(loop) {}

bool Poller::hasChannel(Channel *channel) const {
  auto it = channels_.find(channel->fd());
  return it != channels_.end() && it->second == channel;
}


Poller *Poller::newDefaultPoller(EventLoop *loop) {
  return new EpollPoller(loop);
}

void Poller::assertInLoopThread() { loop_->assertInLoopThread(); }

} // namespace bamboo