#include "base/AsyncLogging.h"

#include "base/LogFile.h"
#include "base/TimeStamp.h"

#include <assert.h>

#include <chrono>

namespace bamboo {

AsyncLogging::AsyncLogging(const std::string &basename, size_t roll_size,
                           int flush_interval)
    : basename_(basename), roll_size_(roll_size),
      flush_interval_(flush_interval),
      thread_(std::bind(&AsyncLogging::threadFunc, this), "Logging") {
  buffers_.reserve(16);
  cur_buffer_->bzero();
  next_buffer_->bzero();
}

void AsyncLogging::append(const char *logline, size_t len) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (cur_buffer_->avail() > len) {
    cur_buffer_->append(logline, len);
  } else {
    buffers_.push_back(std::move(cur_buffer_));

    if (next_buffer_) {
      cur_buffer_ = std::move(next_buffer_);
    } else {
      cur_buffer_.reset(new Buffer); // rarely happen
    }
    cur_buffer_->append(logline, len);
  }
}

void AsyncLogging::threadFunc() {
  assert(running_);
  latch.countDown();
  LogFile output(basename_, roll_size_);
  BufferPtr new_buffer1(new Buffer);
  BufferPtr new_buffer2(new Buffer);
  new_buffer1->bzero();
  new_buffer2->bzero();

  BufferVector buffers_to_write;
  buffers_to_write.reserve(16);

  while (running_) {
    assert(new_buffer1 && new_buffer1->length() == 0);
    assert(new_buffer2  && new_buffer2->length() == 0);
    assert(buffers_to_write.empty());

    {
      std::unique_lock<std::mutex> lock(mutex_);
      if (buffers_.empty()) {
        cond_.wait_for(lock, std::chrono::seconds(flush_interval_));
      }
      buffers_.push_back(std::move(cur_buffer_));
      cur_buffer_ = std::move(new_buffer1);
      buffers_to_write.swap(buffers_);
      if (!next_buffer_) {
        next_buffer_ = std::move(new_buffer2);
      }
    }

    assert(!buffers_to_write.empty());
    if (buffers_to_write.size() > 25) {
      char buf[256];
      snprintf(buf, sizeof(buf), "Drop log message at %s, %zd larger buffers\n",
               TimeStamp::now().toString().c_str(),
               buffers_to_write.size() - 2);
      fputs(buf, stderr);
      output.append(buf, strlen(buf));
      buffers_to_write.erase(buffers_to_write.begin() + 2,
                             buffers_to_write.end());
    }

    for (const auto &buffer : buffers_to_write) {
      output.append(buffer->data(), buffer->length());
    }

    if (buffers_to_write.size() > 2) {
      buffers_to_write.resize(2);
    }

    if (new_buffer1 == nullptr) {
      assert(!buffers_to_write.empty());
      new_buffer1 = std::move(buffers_to_write.back());
      buffers_to_write.pop_back();
      new_buffer1->reset();
    }

    if (new_buffer2 == nullptr) {
      assert(!buffers_to_write.empty());
      new_buffer2 = std::move(buffers_to_write.back());
      buffers_to_write.pop_back();
      new_buffer2->reset();
    }
    buffers_to_write.clear();
    output.flush();
  }
  output.flush();
}
} // namespace bamboo