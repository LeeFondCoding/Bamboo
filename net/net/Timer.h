#pragma once
#include "base/Macro.h"
#include "base/TimeStamp.h"
#include "net/CallBack.h"

namespace bamboo {

// timer
class Timer {
public:
  Timer(TimerCallback callback, TimeStamp when, double interval)
      : callback_(callback), expiration_(when), interval_(interval),
        repeat_(interval > 0.0), sequece_(++num_created_) {}

  DISALLOW_COPY(Timer)

  // call back function
  void run() const { callback_(); }

  // return when it will be triggered
  TimeStamp expiration() const { return expiration_; }

  // whether repeat
  bool repeat() const { return repeat_; }

  // return sequence number
  int64_t sequece() const { return sequece_; }

  void restart(TimeStamp now);

  // number of timers
  static int64_t numCreated() { return num_created_; }

private:
  const TimerCallback callback_;
  TimeStamp expiration_;
  const double interval_;
  const bool repeat_;
  const int64_t sequece_;

  static int64_t num_created_;
};

} // namespace bamboo