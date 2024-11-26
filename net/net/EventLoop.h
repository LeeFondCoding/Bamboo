#pragma once

#include "base/Macro.h"
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

  DISALLOW_COPY(EventLoop)

  ~EventLoop();

  // loop forever
  void loop();
  void quit();
  TimeStamp pollReturnTime() const;

  // Runs callback immediately in the loop thread.
  // It wakes up the loop, and run the cb.
  // If in the same loop thread, cb is run within the function.
  // Safe to call from other threads.
  void runInLoop(Functor func);

  // Queues callback in the loop thread.
  // Runs after finish pooling.
  // Safe to call from other threads.
  void queueInLoop(Functor func);

  // at some time point run timer
  TimerId runAt(const TimeStamp &time, TimerCallback cb);
  // after delay run timer
  TimerId runAfter(double delay, TimerCallback cb);
  // every interval run timer
  TimerId runEvery(double interval, TimerCallback cb);
  // cancel timer
  void cancel(TimerId timerId);

  // wake up this sleeping loop/thread
  void wakeup();

  void updateChannel(Channel *channel);
  void removeChannel(Channel *channel);
  bool hasChannel(Channel *channel);
  
  // abort if not in corresponding thread
  void assertInLoopThread();
  bool isInLoopThread() const;

private:
  // for wake up event
  void handleRead();
  void doPendingFunctors();

  std::atomic<bool> looping_{false};
  std::atomic<bool> quit_{false};
  std::atomic<bool> calling_pending_functors_{false};

  pid_t tid_;
  TimeStamp pool_return_time_;
  std::unique_ptr<Poller> poller_;

  std::unique_ptr<TimerQueue> timer_queue_;

  int wakeup_fd_;
  std::unique_ptr<Channel> wakeup_channel_;

  std::vector<Channel *> active_channels_;
  Channel *current_active_channel_;

  std::mutex mutex_;
  std::vector<Functor> pending_functors_;// guard by mutex_
};
} // namespace bamboo