#pragma once

#include <cstdint>

#include <string>

namespace bamboo {
class TimeStamp {
public:
  TimeStamp() = default;
  explicit TimeStamp(int64_t microSecondSinceEpoch_)
      : microSecondSinceEpoch_(microSecondSinceEpoch_) {}

  static TimeStamp now();

  //xxxx/xx/xx xx:xx:xx   e.g. 2024/10/12 12:12:12
  std::string toString() const;

private:
  int64_t microSecondSinceEpoch_{0};
};
} // namespace bamboo