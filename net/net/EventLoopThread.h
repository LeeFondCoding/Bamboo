#pragma once

#include "base/Macro.h"
#include "base/Thread.h"

#include <condition_variable>
#include <functional>
#include <mutex>
#include <string>

namespace bamboo {

class EventLoop;

class EventLoopThread {
public:
  using ThreadInitCallback = std::function<void(EventLoop *)>;

  EventLoopThread(const ThreadInitCallback &cb = ThreadInitCallback(),
                  const std::string &name = std::string());

  DISALLOW_COPY(EventLoopThread)

  ~EventLoopThread();

  EventLoop *startLoop();

private:
  void threadFunc();

  bool exiting_{false};
  EventLoop *loop_{nullptr};
  Thread thread_;
  std::mutex mutex_{};
  std::condition_variable cond_{};
  ThreadInitCallback callback_;
};

} // namespace bamboo