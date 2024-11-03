#pragma once

#include "base/Macro.h"

#include <stddef.h>
#include <stdio.h>

namespace bamboo {

// interface to file
class FileUtil {
public:
  DISALLOW_COPY(FileUtil);

  explicit FileUtil(const char *filename);

  // close fd
  ~FileUtil();

  // not thread safe
  void append(const char *logline, size_t len);

  // flush buffer to file
  void flush();

  // record written bytes
  size_t writtenBytes() const { return written_bytes_; }

private:
  size_t write(const char *logline, size_t len);

  // append and clo-exec
  FILE *fp_;
  // record has written bytes
  size_t written_bytes_{0};

  char buffer_[64 * 1024];
};

} // namespace bamboo