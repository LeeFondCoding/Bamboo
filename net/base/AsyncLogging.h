#pragma once

#include "base/CountDownLatch.h"
#include "base/LogStream.h"
#include "base/Thread.h"

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <vector>

namespace bamboo {

class AsyncLogging {
public:
  AsyncLogging(const std::string &basename, size_t roll_size,
               int flush_interval = 3);

  DISALLOW_COPY(AsyncLogging)

  ~AsyncLogging() {
    if (running_) {
      stop();
    }
  }

  void append(const char *logline, size_t len);

  void start() {
    running_ = true;
    thread_.start();
    latch.wait();
  }

  void stop() {
    running_ = false;
    cond_.notify_all();
    thread_.join();
  }

private:
  void threadFunc();

  using Buffer = FixedBuffer<kSmallBuffer>;
  using BufferPtr = std::unique_ptr<Buffer>;
  using BufferVector = std::vector<BufferPtr>;

  const int flush_interval_;
  std::atomic<bool> running_{false};
  const std::string basename_;
  const size_t roll_size_;
  Thread thread_;
  CountDownLatch latch{1};
  std::mutex mutex_;
  std::condition_variable cond_; // Guarded by mutex_
  BufferPtr cur_buffer_{new Buffer};         // Guarded by mutex_
  BufferPtr next_buffer_{new Buffer};        // Guarded by mutex_
  BufferVector buffers_{};         // Guarded by mutex_
};

} // namespace bamboo