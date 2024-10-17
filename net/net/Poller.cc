#include "net/Poller.h"

#include "net/Channel.h"

namespace bamboo {

Poller::Poller(EventLoop *loop) : loop_(loop) {}

bool Poller::hasChannel(Channel *channel) const {
  auto it = channel_map_.find(channel->fd());
  return it != channel_map_.end() && it->second == channel;
}

} // namespace bamboo