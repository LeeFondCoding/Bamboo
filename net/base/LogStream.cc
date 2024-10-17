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

LogStream &LogStream::operator<<(const short val) {
  return *this << static_cast<int>(val);
}

LogStream &LogStream::operator<<(const unsigned short val) {
  return *this << static_cast<unsigned int>(val);
}

LogStream &LogStream::operator<<(const int val) {
  formatInteger(val);
  return *this;
}

LogStream &LogStream::operator<<(const unsigned int val) {
  formatInteger(val);
  return *this;
}

LogStream &LogStream::operator<<(const long val) {
  formatInteger(val);
  return *this;
}

LogStream &LogStream::operator<<(const unsigned long val) {
  formatInteger(val);
  return *this;
}

LogStream &LogStream::operator<<(const long long val) {
  formatInteger(val);
  return *this;
}

LogStream &LogStream::operator<<(const unsigned long long val) {
  formatInteger(val);
  return *this;
}

LogStream &LogStream::operator<<(const char ch) {
  buffer_.append(&ch, 1);
  return *this;
}

LogStream &LogStream::operator<<(const void *p) {
  uintptr_t v = reinterpret_cast<uintptr_t>(p);
  if (buffer_.avail() >= kMaxNumericSize) {
    char *buf = buffer_.current();
    buf[0] = '0';
    buf[1] = 'x';
    size_t len = convert(buf + 2, v);
    buffer_.add(len + 2);
  }
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

LogStream &LogStream::operator<<(const unsigned char *str) {
  return *this << reinterpret_cast<const char *>(str);
}

LogStream &LogStream::operator<<(const std::string &str) {
  buffer_.append(str.c_str(), str.size());
  return *this;
}

template <typename T> void LogStream::formatInteger(T val) {
  if (buffer_.avail() >= kMaxNumericSize) {
    size_t len = convert(buffer_.current(), val);
    buffer_.add(len);
  }
}

} // namespace bamboo