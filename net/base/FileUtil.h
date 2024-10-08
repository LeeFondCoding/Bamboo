#pragma once

#include "Macro.h"
#include <cstddef>
#include <cstdio>

namespace bamboo {

class AppendFile {
public:
  DISALLOW_COPY(AppendFile);

  explicit AppendFile(const char *filename);

  ~AppendFile();

  // not thread safe
  void append(const char *logline, size_t len);

  void flush();

  size_t writtenBytes() const { return written_bytes_; }

private:
  size_t write(const char *logline, size_t len);

  // append and O_CLOEXEC
  FILE *fp_;
  // record has written bytes
  size_t written_bytes_{0};

  char buffer_[64 * 1024];
};

} // namespace bamboo