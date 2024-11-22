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

  virtual ~Poller() = default;

  virtual TimeStamp poll(int timeoutMs, ChannelList *activeChannels) = 0;

  virtual void updateChannel(Channel *channel) = 0;

  virtual void removeChannel(Channel *channel) = 0;

  bool hasChannel(Channel *channel) const;

  // return Poller should RAII
  static Poller *newDefaultPoller(EventLoop *loop);

  void assertInLoopThread();

protected:
  using ChannelMap = std::unordered_map<int, Channel *>;
  
  ChannelMap channels_;

private:
  EventLoop *loop_;
};
} // namespace bamboo