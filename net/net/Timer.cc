#include "net/Timer.h"

namespace bamboo {

int64_t Timer::num_created_ = 0;

void Timer::restart(TimeStamp now) {
  if (repeat_) {
    expiration_ = addTime(now, interval_);
  } else {
    expiration_ = TimeStamp::getInvalid();
  }
}

} // namespace bamboo