#include "base/CountDownLatch.h"

namespace bamboo {
CountDownLatch::CountDownLatch(int count)
    : mutex_(), con_var_(), count_(count) {}

void CountDownLatch::wait() {
  std::unique_lock<std::mutex> lock(mutex_);
  con_var_.wait(lock, [&]() { return count_ == 0; });
}

void CountDownLatch::countDown() {
  std::unique_lock<std::mutex> lock(mutex_);
  if (--count_ == 0) {
    con_var_.notify_all();
  }
}

int CountDownLatch::getCount() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return count_;
}

} // namespace bamboo