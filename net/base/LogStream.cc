#include "base/LogStream.h"

#include <algorithm>

namespace {
constexpr char digits[] = "9876543210123456789";
const char *zero = digits + 9;

template <typename T> size_t convert(char buf[], T value) {
  T i = value;
  char *p = buf;

  do {
    int lsd = static_cast<int>(i % 10);
    i /= 10;
    *p++ = zero[lsd];
  } while (i != 0);

  if (value < 0) {
    *p++ = '-';
  }

  std::reverse(buf, p);
  return p - buf;
}

} // namespace

namespace bamboo {

LogStream &LogStream::operator<<(const bool val) {
  if (val) {
    buffer_.append("(true)", 6);
  } else {
    buffer_.append("(false)", 7);
  }
  return *this;
}

LogStream &LogStream::operator<<(const int val) {
  formatInteger(val);
  return *this;
}

LogStream &LogStream::operator<<(const unsigned int val) {
  formatInteger(val);
  return *this;
}

LogStream &LogStream::operator<<(const char *str) {
  if (str != nullptr) {
    buffer_.append(str, strlen(str));
  } else {
    buffer_.append("(null)", 6);
  }
  return *this;
}

template <typename T> void LogStream::formatInteger(T val) {
  if (buffer_.avail() >= kMaxNumericSize) {
    size_t len = convert(buffer_.current(), val);
    buffer_.add(len);
  }
}

} // namespace bamboo