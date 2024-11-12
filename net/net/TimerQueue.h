#pragma once

#include "base/Macro.h"
#include "base/TimeStamp.h"
#include "net/CallBack.h"
#include "net/Channel.h"

#include <set>
#include <vector>

namespace bamboo {

class EventLoop;
class Timer;
class TimerId;

class TimerQueue {
public:
explicit TimerQueue(EventLoop *loop);

DISALLOW_COPY(TimerQueue)

~TimerQueue();

TimerId addTimer(TimerCallback cb, TimeStamp when, double interval);

void cancel(TimerId timerId);

private:
  using Entry = std::pair<TimeStamp, Timer *>;
  using TimerList = std::set<Entry>;
  using ActiveTimer = std::pair<Timer *, int64_t>;
  using ActiveTimerSet = std::set<ActiveTimer>;

  void addTimerInLoop(Timer *timer);

  void cancelInLoop(TimerId timerId);

  void handleRead();

  std::vector<Entry> getExpired(TimeStamp now);

  void reset(const std::vector<Entry> &expired, TimeStamp now);

  bool insert(Timer *timer);

  bool calling_expired_timers_{false};
  const int timerfd_;
  EventLoop *loop_;
  Channel timerfd_channel_;
  TimerList timers_;

  ActiveTimerSet active_timers_;
  ActiveTimerSet canceling_timers_;
};

} // namespace bamboo