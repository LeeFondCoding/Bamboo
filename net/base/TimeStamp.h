#pragma once

#include <stdint.h>

#include <string>

namespace bamboo {

class TimeStamp {
public:
  TimeStamp() = default;

  explicit TimeStamp(int64_t microSecondSinceEpoch_)
      : microSecondSinceEpoch_(microSecondSinceEpoch_) {}

  // return micro seconds since epoch
  int64_t microSecondsSinceEpoch() const { return microSecondSinceEpoch_; }

  void swap(TimeStamp &that) {
    std::swap(microSecondSinceEpoch_, that.microSecondSinceEpoch_);
  }

  // e.g. 2024/10/12 12:12:12
  std::string toString() const;

  bool isValid() const { return microSecondSinceEpoch_ > 0; }

  // return current time
  static TimeStamp now();

  // return invalid timestamp
  static TimeStamp getInvalid() { return TimeStamp(); }

  static constexpr int kMicroSecondsPerSecond = 1000 * 1000;

private:
  int64_t microSecondSinceEpoch_{0};
};

inline bool operator<(TimeStamp lhs, TimeStamp rhs) {
  return lhs.microSecondsSinceEpoch() < rhs.microSecondsSinceEpoch();
}

inline bool operator==(TimeStamp lhs, TimeStamp rhs) {
  return lhs.microSecondsSinceEpoch() == rhs.microSecondsSinceEpoch();
}

inline TimeStamp addTime(TimeStamp timestamp, double seconds) {
  int64_t delta =
      static_cast<int64_t>(seconds * TimeStamp::kMicroSecondsPerSecond);
  return TimeStamp(timestamp.microSecondsSinceEpoch() + delta);
}

} // namespace bamboo