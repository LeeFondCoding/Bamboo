#pragma once

#include "net/Poller.h"

struct epoll_event;

namespace bamboo {

class EpollPoller : public Poller {
public:
EpollPoller(EventLoop *loop);

~EpollPoller() override;

TimeStamp poll(int timeoutMs, ChannelList *activeChannels) override;

void removeChannel(Channel *channel) override;

void updateChannel(Channel *channel) override;

private:
  using EventList = std::vector<struct epoll_event>;

  static const int kInitEventListSize = 16;

  static const char *operationToString(int op);

  void fillActiveChannels(int numEvents, ChannelList *activeChannels) const;

  void update(int operation, Channel *channel);

  int epollfd_;
  EventList events_;
};
} // namespace bamboo