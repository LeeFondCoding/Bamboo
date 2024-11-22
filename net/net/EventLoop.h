#pragma once

#include "base/TimeStamp.h"
#include "net/CallBack.h"
#include "net/TimerId.h"

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

namespace bamboo {

class Channel;
class Poller;
class TimerQueue;

class EventLoop {
public:
  using Functor = std::function<void()>;

  EventLoop();

  ~EventLoop();
  void loop();
  void quit();
  TimeStamp pollReturnTime() const;
  void runInLoop(Functor func);
  void queueInLoop(Functor func);

  // at some time point run timer
  TimerId runAt(const TimeStamp &time, TimerCallback cb);
  // after delay run timer
  TimerId runAfter(double delay, TimerCallback cb);
  // every interval run timer
  TimerId runEvery(double interval, TimerCallback cb);

  void wakeup();

  void updateChannel(Channel *channel);
  void removeChannel(Channel *channel);
  bool hasChannel(Channel *channel);
  
  void assertInLoopThread();
  bool isInLoopThread() const;

private:
  void handleRead();
  void doPendingFunctors();

  std::atomic<bool> looping_{false};
  std::atomic<bool> quit_{false};
  std::atomic<bool> calling_pending_functors{false};

  pid_t tid_;
  TimeStamp pool_return_time_;
  std::unique_ptr<Poller> poller_;

  std::unique_ptr<TimerQueue> timer_queue_;

  int wakeup_fd_;
  std::unique_ptr<Channel> wakeup_channel_;
  std::vector<Channel *> active_channels_;
  Channel *current_active_channel_;
  std::vector<Functor> pending_functors_;
  std::mutex mutex_;
};
} // namespace bamboo