#pragma once

#include "Macro.h"

#include <cstddef>

#include <memory>
#include <string>

namespace bamboo {

class AppendFile;

class LogFile {
  LogFile(const std::string &basename, size_t roll_size, int flush_interval = 3,
          int check_every_n = 1024);

  ~LogFile() = default;

  DISALLOW_COPY(LogFile)

  void append(const char *logline, size_t len);

  void flush();

  void rollFile();

private:
  static std::string getLogFileName(const std::string &basename);

  const std::string basename_;
  const size_t roll_size_;
  const int flush_interval_;
  const int check_every_n_;

  int count_{0};
  time_t start_{0};
  time_t last_roll_{0};
  time_t last_flush_{0};
  std::unique_ptr<AppendFile> file_;

  const static int kRollPerSeconds_ = 60 * 60 * 24;
};
} // namespace bamboo