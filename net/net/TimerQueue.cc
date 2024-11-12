#pragma once

#include "net/TimerQueue.h"

#include "base/Logging.h"
#include "net/EventLoop.h"
#include "net/Timer.h"
#include "net/TimerId.h"

#include "assert.h"
#include <sys/timerfd.h>
#include <unistd.h>

namespace {

// create a timer for time interval, non blocking and close exec
int createTimefd() {
  int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
  if (timerfd < 0) {
    bamboo::LOG_SYSFATAL << "Failed in timerfd_create";
  }
  return timerfd;
}

// compute time diff from when to now; less than 100ms, set to 100ms
struct timespec howMuchTimeFromNow(bamboo::TimeStamp when) {
  int64_t microseconds = when.microSecondsSinceEpoch() -
                         bamboo::TimeStamp::now().microSecondsSinceEpoch();
  if (microseconds < 100) {
    microseconds = 100;
  }

  struct timespec ts;
  ts.tv_sec = static_cast<time_t>(microseconds /
                                  bamboo::TimeStamp::kMicroSecondsPerSecond);
  ts.tv_nsec = static_cast<long>(
      (microseconds % bamboo::TimeStamp::kMicroSecondsPerSecond) * 1000);
  return ts;
}

void readTimerfd(int timerfd, bamboo::TimeStamp now) {
  uint64_t howmany;
  auto n = ::read(timerfd, &howmany, sizeof howmany);
  if (n != sizeof howmany) {
    bamboo::LOG_ERROR << "TimerQueue::handleRead() reads " << n
                      << " bytes instead of 8";
  }
}

void resetTimerfd(int timerfd, bamboo::TimeStamp expiration) {
  struct itimerspec old_value;
  struct itimerspec new_value;
  memset(&old_value, 0, sizeof(old_value));
  memset(&new_value, 0, sizeof(new_value));
  new_value.it_value = howMuchTimeFromNow(expiration);
  int ret = ::timerfd_settime(timerfd, 0, &new_value, &old_value);
  if (ret) {
    bamboo::LOG_SYSERR << "timerfd_settime()";
  }
}

} // namespace

namespace bamboo {

TimerQueue::TimerQueue(EventLoop *loop)
    : loop_(loop), timerfd_(createTimefd()), timerfd_channel_(loop, timerfd_) {
  timerfd_channel_.setReadCallback(std::bind(&TimerQueue::handleRead, this));
  timerfd_channel_.enableReading();
}

TimerQueue::~TimerQueue() {
  timerfd_channel_.disableAll();
  timerfd_channel_.remove();
  ::close(timerfd_);
  for (const Entry &timer : timers_) {
    delete timer.second;
  }
}

TimerId TimerQueue::addTimer(TimerCallback cb, TimeStamp when,
                             double interval) {
  auto timer = new Timer(std::move(cb), when, interval);
  loop_->runInLoop(std::bind(&TimerQueue::addTimerInLoop, this, timer));
  return {timer, timer->sequece()};
}

void TimerQueue::cancel(TimerId timerId) {
  loop_->runInLoop(std::bind(&TimerQueue::cancelInLoop, this, timerId));
}

void TimerQueue::addTimerInLoop(Timer *timer) {
  loop_->assertInLoopThread();
  bool ret = insert(timer);
  if (ret) {
    resetTimerfd(timerfd_, timer->expiration());
  }
}

void TimerQueue::cancelInLoop(TimerId timerId) {
  loop_->assertInLoopThread();
  ActiveTimer timer{timerId.timer_, timerId.seq_};
  auto iter = active_timers_.find(timer);
  if (iter != active_timers_.end()) {
    auto n = timers_.erase({iter->first->expiration(), iter->first});
    delete iter->first;
    active_timers_.erase(iter);
  } else if (calling_expired_timers_) {
    canceling_timers_.insert(timer);
  }
  assert(timers_.size() == active_timers_.size());
}

void TimerQueue::handleRead() {
  loop_->assertInLoopThread();
  TimeStamp now(TimeStamp::now());
  readTimerfd(timerfd_, now);
  std::vector<Entry> expired = getExpired(now);

  calling_expired_timers_ = true;
  canceling_timers_.clear();

  for (const auto &it : expired) {
    it.second->run();
  }
  calling_expired_timers_ = false;

  reset(expired, now);
}

std::vector<TimerQueue::Entry> TimerQueue::getExpired(TimeStamp now) {
  assert(timers_.size() == active_timers_.size());
  std::vector<Entry> expired;
  Entry sentry(now, reinterpret_cast<Timer *>(UINTPTR_MAX));
  auto end = timers_.lower_bound(sentry);
  std::copy(timers_.begin(), end, back_inserter(expired));
  timers_.erase(timers_.begin(), end);
  // erase expired timers from active timers
  for (const auto &iter : expired) {
    ActiveTimer timer(iter.second, iter.second->sequece());
    active_timers_.erase(timer);
  }
  return expired;
}

void TimerQueue::reset(const std::vector<Entry> &expired, TimeStamp now) {
  TimeStamp next_expire;
  for (const auto &it : expired) {
    if (it.second->repeat()) {
      it.second->restart(now);
      insert(it.second);
    } else {
      delete it.second;
    }
  }

  if (!timers_.empty()) {
    next_expire = timers_.begin()->second->expiration();
  }
  if (next_expire.isValid()) {
    resetTimerfd(timerfd_, next_expire);
  }
}

bool TimerQueue::insert(Timer *timer) {
  loop_->assertInLoopThread();
  assert(timers_.size() == active_timers_.size());
  bool earliest_changed = false;
  auto when = timer->expiration();
  auto iter = timers_.begin();
  if (iter == timers_.end() || when < iter->first) {
    earliest_changed = true;
  }
  timers_.emplace(std::make_pair(when, timer));
  active_timers_.emplace(std::make_pair(timer, timer->sequece()));

  return earliest_changed;
}

} // namespace bamboo