#include "net/EventLoop.h"

#include "base/CurrentThread.h"
#include "base/Logging.h"
#include "net/Channel.h"
#include "net/Poller.h"

#include <assert.h>
#include <sys/eventfd.h>

namespace bamboo {

namespace {

thread_local EventLoop *t_loopInThisThread = nullptr;

constexpr int kPollTimeMs = 1000;

int creatEventfd() {
  int fd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  if (fd < 0) {
    LOG_SYSFATAL << "failed in eventfd";
  }
  return fd;
}
} // namespace

EventLoop::EventLoop()
    : tid_(CurrentThread::tid()), poller_(Poller::newDefaultPoller(this)),
      wakeup_fd_(creatEventfd()),
      wakeup_channel_(new Channel(this, wakeup_fd_)),
      current_active_channel_(nullptr) {
  LOG_DEBUG << "EventLoop created " << this << " in thread " << tid_;
  wakeup_channel_->setReadCallback(std::bind(&EventLoop::handleRead, this));
  if (t_loopInThisThread) {
    LOG_FATAL << "Another EventLoop " << t_loopInThisThread
              << " exists in this thread " << tid_;
  } else {
    t_loopInThisThread = this;
  }

  wakeup_channel_->setReadCallback(std::bind(&EventLoop::handleRead, this));
  wakeup_channel_->enableReading();
}

EventLoop::~EventLoop() {
  LOG_DEBUG << "EventLoop " << this << " of thread " << tid_
            << " destructs in thread " << CurrentThread::tid();
  wakeup_channel_->disableAll();
  wakeup_channel_->remove();
  ::close(wakeup_fd_);
  t_loopInThisThread = nullptr;
}

void EventLoop::loop() {
  assert(!looping_);
  assertInLoopThread();
  looping_ = true;
  quit_ = false;
  LOG_TRACE << "EventLoop " << this << " start looping";
  while (!quit_) {
    active_channels_.clear();
    pool_return_time_ = poller_->poll(kPollTimeMs, &active_channels_);
    for (auto chan : active_channels_) {
      current_active_channel_ = chan;
      current_active_channel_->handleEvent(pool_return_time_);
    }
    current_active_channel_ = nullptr;
    doPendingFunctors();
  }

  LOG_TRACE << "EventLoop " << this << " stop looping";
  looping_ = false;
}

void EventLoop::quit() {
  quit_ = true;
  if (!isInLoopThread()) {
    wakeup();
  }
}

TimeStamp EventLoop::pollReturnTime() const { return pool_return_time_; }

void EventLoop::runInLoop(Functor func) {
  if (isInLoopThread()) {
    func();
  } else {
    queueInLoop(std::move(func));
  }
}

void EventLoop::wakeup() {
  uint64_t one = 1;
  ssize_t n = write(wakeup_fd_, &one, sizeof one);
  if (n != sizeof(one)) {
    LOG_SYSERR << "EventLoop::wakeup() writes " << n << " bytes instead of "
               << sizeof(one);
  }
}

void EventLoop::updateChannel(Channel *channel) {
  poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel *channel) {
  poller_->removeChannel(channel);
}

bool EventLoop::hasChannel(Channel *channel) {
  return poller_->hasChannel(channel);
}

void EventLoop::assertInLoopThread() {
  if (!isInLoopThread()) {
    LOG_FATAL << "EventLoop::assertInLoopThread - EventLoop " << this
              << " was created in threadId_ = " << tid_
              << ", current thread id = " << CurrentThread::tid();
  }
}

bool EventLoop::isInLoopThread() const { return CurrentThread::tid() == tid_; }

void EventLoop::handleRead() {
  uint64_t one = 1;
  ssize_t n = read(wakeup_fd_, &one, sizeof one);
  if (n != sizeof(one)) {
    LOG_SYSERR << "EventLoop::handleRead() reads " << n << " bytes instead of "
               << sizeof(one);
  }
}

void EventLoop::doPendingFunctors() {
  std::vector<Functor> functors;
  {
    std::lock_guard<std::mutex> guard(mutex_);
    functors.swap(pending_functors_);
  }

  for (const auto &func : functors) {
    func();
  }
}
} // namespace bamboo