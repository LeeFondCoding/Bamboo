#pragma once

#include "base/Macro.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include <string>

namespace bamboo {

constexpr int kSmallBuffer = 4000;
constexpr int kLargeBuffer = 4000 * 1000;

template <int SIZE> class FixedBuffer {
public:
  FixedBuffer() : cur_(data_) {}

  void append(const char *buf, size_t len) {
    if (avail() > len) {
      memcpy(cur_, buf, len);
      cur_ += len;
    } else {
      fprintf(stderr, "buffer OVERFLOW\n");
    }

  }

  size_t length() const { return cur_ - data_; }

  char *current() { return cur_; }

  size_t avail() const { return end() - cur_; }

  void add(size_t len) { cur_ += len; }

  void reset() { cur_ = data_; }

  void bzero() { memset(data_, 0, sizeof(data_)); }

  const char *data() const { return data_; }

private:
  const char *end() const { return data_ + sizeof(data_); }

  char data_[SIZE];
  char *cur_;
};

class LogStream {
public:
  using Buffer = FixedBuffer<kSmallBuffer>;

  // TODO: implement rest of the operators
  LogStream &operator<<(const bool);
  LogStream &operator<<(const short);
  LogStream &operator<<(const unsigned short);
  LogStream &operator<<(const int);
  LogStream &operator<<(const unsigned int);
  LogStream &operator<<(const long);
  LogStream &operator<<(const unsigned long);
  LogStream &operator<<(const long long);
  LogStream &operator<<(const unsigned long long);
  LogStream &operator<<(const char);
  LogStream &operator<<(const void *);
  LogStream &operator<<(const char *);
  LogStream &operator<<(const unsigned char *);
  LogStream &operator<<(const std::string &);

  void append(const char *data, size_t len) { buffer_.append(data, len); }

  const Buffer &buffer() const { return buffer_; }

private:
  template <typename T> void formatInteger(T);

  Buffer buffer_;
  constexpr static int kMaxNumericSize = 32;
};

} // namespace bamboo