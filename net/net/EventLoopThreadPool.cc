#include "net/EventLoopThreadPool.h"

namespace bamboo {

EventLoopThreadPool::EventLoopThreadPool(EventLoop *baseLoop,
                                         const std::string &name)
    : base_loop_(baseLoop), name_(name) {}

void EventLoopThreadPool::start(const ThreadInitCallback &cb) {
  started_ = true;
  for (int i = 0; i < threads_num_; i++) {
    char buf[name_.size() + 32];
    snprintf(buf, sizeof buf, "%s%d", name_.c_str(), i);
    auto loop_thread = new EventLoopThread(cb, buf);
    loops_.emplace_back(loop_thread->startLoop());
    threads_.emplace_back(std::unique_ptr<EventLoopThread>(loop_thread));
  }

  if (threads_num_ == 0) {
    cb(base_loop_);
  }
}

EventLoop *EventLoopThreadPool::getNextLoop() {
  auto loop = base_loop_;
  if (!loops_.empty()) {
    loop = loops_[next_];
    next_ = (next_ + 1) % loops_.size();
  }
  return loop;
}

} // namespace bamboo