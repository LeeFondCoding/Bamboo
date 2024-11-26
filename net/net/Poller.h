#pragma once

#include "base/Macro.h"
#include "base/TimeStamp.h"

#include <unordered_map>
#include <vector>

namespace bamboo {

class Channel;
class EventLoop;

class Poller {
public:
  using ChannelList = std::vector<Channel *>;

  Poller(EventLoop *loop);

  DISALLOW_COPY(Poller)

  virtual ~Poller();

  virtual TimeStamp poll(int timeout_ms, ChannelList *active_channels) = 0;

  virtual void updateChannel(Channel *channel) = 0;

  virtual void removeChannel(Channel *channel) = 0;

  bool hasChannel(Channel *channel) const;

  // return Poller should RAII
  static Poller *newDefaultPoller(EventLoop *loop);

  void assertInLoopThread() const;

protected:
  using ChannelMap = std::unordered_map<int, Channel *>;

  ChannelMap channels_;

private:
  EventLoop *loop_;
};
} // namespace bamboo