#include <net/EpollPoller.h>

#include "base/Logging.h"
#include "net/Channel.h"

#include <assert.h>
#include <sys/epoll.h>
#include <unistd.h>

namespace {
constexpr int kNew = -1;
constexpr int kAdded = 1;
constexpr int kDeleted = 2;
} // namespace

namespace bamboo {

EpollPoller::EpollPoller(EventLoop *loop)
    : Poller(loop), epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
      events_(kInitEventListSize) {
  if (epollfd_ < 0) {
    LOG_SYSFATAL << "EpollPoller::EpollPoller";
  }
}

EpollPoller::~EpollPoller() { ::close(epollfd_); }

TimeStamp EpollPoller::poll(int timeoutMs, ChannelList *active_channels) {
  LOG_TRACE << "fd total count " << channels_.size();
  int num_events =
      ::epoll_wait(epollfd_, &*events_.begin(), events_.size(), timeoutMs);
  int saved_err = errno;
  auto now = TimeStamp::now();
  if (num_events > 0) {
    LOG_TRACE << num_events << " events happened";
    fillActiveChannels(num_events, active_channels);
    if (num_events == events_.size()) {
      events_.resize(events_.size() * 2);
    }
  } else if (num_events == 0) {
    LOG_TRACE << "nothing happend";
  } else {
    if (saved_err != EINTR) {
      errno = saved_err;
      LOG_SYSERR << "EpollPoller::poll";
    }
  }
  return now;
}

void EpollPoller::fillActiveChannels(int numEvents,
                                     ChannelList *active_channels) const {
  for (int i = 0; i < numEvents; ++i) {
    auto channel = static_cast<Channel *>(events_[i].data.ptr);
    channel->setRevents(events_[i].events);
    active_channels->push_back(channel);
  }
}

void EpollPoller::updateChannel(Channel *channel) {
  Poller::assertInLoopThread();
  const int index = channel->index();
  LOG_TRACE << "fd = " << channel->fd() << " events = " << channel->events()
            << " index = " << index;
  if (index == kNew || index == kDeleted) {
    int fd = channel->fd();
    if (index == kNew) {
      assert(channels_.find(fd) == channels_.end());
      channels_[fd] = channel;
    } else {
      assert(channels_.find(fd) != channels_.end());
    }

    channel->setIndex(kAdded);
    update(EPOLL_CTL_ADD, channel);
  } else {
    assert(channels_.find(channel->fd()) != channels_.end());
    assert(channels_[channel->fd()] == channel);
    assert(index == kAdded);
    if (channel->isNoneEvent()) {
      update(EPOLL_CTL_DEL, channel);
      channel->setIndex(kDeleted);
    } else {
      update(EPOLL_CTL_MOD, channel);
    }
  }
}

void EpollPoller::removeChannel(Channel *channel) {
  Poller::assertInLoopThread();
  int fd = channel->fd();
  LOG_TRACE << "fd = " << fd;
  assert(channels_.find(fd) != channels_.end());
  assert(channels_[fd] == channel);
  assert(channel->isNoneEvent());
  int index = channel->index();
  assert(index == kAdded || index == kDeleted);
  size_t n = channels_.erase(fd);
  assert(1 == n);
  if (index == kAdded) {
    update(EPOLL_CTL_DEL, channel);
  }
  channel->setIndex(kNew);
}

void EpollPoller::update(int operation, Channel *channel) {
  struct epoll_event event;
  memset(&event, 0, sizeof(event));
  event.events = channel->events();
  event.data.ptr = channel;
  int fd = channel->fd();
  auto operation_str = operationToString(operation);
  LOG_TRACE << "epoll_ctl op = " << operation_str << " fd = " << fd
            << " events = {" << channel->eventsToString() << "}";
  if (::epoll_ctl(epollfd_, operation, fd, &event) < 0) {
    if (operation == EPOLL_CTL_DEL) {
      LOG_SYSERR << "epoll_ctl op = " << operation_str << " fd = " << fd;
    } else {
      LOG_SYSFATAL << "epoll_ctl op = " << operation_str << " fd = " << fd;
    }
  }
}

const char *EpollPoller::operationToString(int operation) {
  switch (operation) {
  case EPOLL_CTL_ADD:
    return "ADD";
  case EPOLL_CTL_DEL:
    return "DEL";
  case EPOLL_CTL_MOD:
    return "MOD";
  default:
    assert(false && "ERROR op");
    return "Unknown Operation";
  }
}
} // namespace bamboo