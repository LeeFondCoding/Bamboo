#pragma once

#include "base/Macro.h"
#include "base/FileUtil.h"

#include <cstddef>

#include <memory>
#include <string>

namespace bamboo {

// roll file 
class LogFile {
public:
  LogFile(const std::string &basename, size_t roll_size, int flush_interval = 3,
          int check_every_n = 1024);

  ~LogFile() = default;

  DISALLOW_COPY(LogFile)

  void append(const char *logline, size_t len);

  void flush();

  // replace current log file with new one
  void rollFile();

private:
  // return log file name with time e.g. basename.20241027-195722.log
  static std::string getLogFileName(const std::string &basename);

  const std::string basename_;
  const size_t roll_size_;
  const int flush_interval_;
  const int check_every_n_;

  int count_{0};
  time_t start_{0};
  time_t last_roll_{0};
  time_t last_flush_{0};
  std::unique_ptr<FileUtil> file_;

  const static int kRollPerSeconds_ = 60 * 60 * 24;
};
} // namespace bamboo