#include "base/Thread.h"

#include "base/CountDownLatch.h"
#include "base/CurrentThread.h"

namespace bamboo {

std::atomic<int> Thread::thread_num_{0};

Thread::Thread(ThreadFunc func, const std::string &name)
    : func_(func), name_(name) {
  setDefaultName();
}

Thread::~Thread() {
  if (started_ && !joined_) {
    thread_->detach();
  }
}

void Thread::start() {
  started_ = true;
  CountDownLatch latch(1);
  thread_ = std::unique_ptr<std::thread>(new std::thread([&]() {
    tid_ = CurrentThread::tid();
    latch.countDown();
    func_();
  }));

  latch.wait();
}

void Thread::join() {
  joined_ = true;
  thread_->join();
}

void Thread::setDefaultName() {
  int num = ++thread_num_;
  if (name_.empty()) {
    char buf[32];
    snprintf(buf, sizeof buf, "Thread%d", num);
    name_ = buf;
  }
}

} // namespace bamboo