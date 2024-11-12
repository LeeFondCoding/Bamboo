#pragma once

#include <cstdint>

namespace bamboo {

class Timer;

// timer id
class TimerId {
public:
  TimerId() = default;

  TimerId(Timer *timer, int64_t seq) : timer_(timer), seq_(seq) {}

  friend class TimerQueue;

private:
  Timer *timer_{nullptr};
  int64_t seq_{0};
};

} // namespace bamboo