#pragma once

#include "base/Macro.h"

#include <atomic>
#include <functional>
#include <memory>
#include <string>
#include <thread>

namespace bamboo {

// wrapper std::thread
class Thread {
public:
  using ThreadFunc = std::function<void()>;

  explicit Thread(ThreadFunc func, const std::string &name = std::string());

  DISALLOW_COPY(Thread);

  ~Thread();

  void start();

  void join();

  bool isStarted() const { return started_; }

  pid_t tid() const { return tid_; }

  static int threadCreated() { return thread_num_; }

private:
  void setDefaultName();

  bool started_{false};
  bool joined_{false};
  pid_t tid_{0};
  ThreadFunc func_;
  std::string name_;
  std::unique_ptr<std::thread> thread_;

  static std::atomic_int thread_num_;
};

} // namespace bamboo