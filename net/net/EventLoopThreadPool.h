#pragma once

#include "base/Macro.h"
#include "net/EventLoopThread.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace bamboo {

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool {
public:
  using ThreadInitCallback = std::function<void(EventLoop *)>;

  EventLoopThreadPool(EventLoop *baseLoop, const std::string &name);

  DISALLOW_COPY(EventLoopThreadPool)

  ~EventLoopThreadPool() = default;

  void setThreadNum(int threads_num) { threads_num_ = threads_num; }

  void start(const ThreadInitCallback &cb = ThreadInitCallback());

  EventLoop *getNextLoop();

  std::vector<EventLoop *> getAllLoops() {
    return loops_.empty() ? std::vector<EventLoop *>{base_loop_} : loops_;
  }

  bool started() const { return started_; }

  const std::string name() const { return name_; }

private:
  EventLoop *base_loop_;
  std::string name_;
  bool started_{false};
  int threads_num_{0};
  int next_{0};
  std::vector<std::unique_ptr<EventLoopThread>> threads_;
  std::vector<EventLoop *> loops_;
};

} // namespace bamboo