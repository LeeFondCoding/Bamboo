#include "base/LogFile.h"

#include "base/FileUtil.h"

#include <cstdio>
#include <ctime>

namespace bamboo {
LogFile::LogFile(const std::string &basename, size_t roll_size,
                 int flush_interval, int check_every_n)
    : basename_(basename), roll_size_(roll_size),
      flush_interval_(flush_interval), check_every_n_(check_every_n),
      count_(0) {
  rollFile();
}

// if write frequecy >= check_every_n, and time >= flush_interval, roll
//  and written bytes >= roll_size, roll
void LogFile::append(const char *logline, size_t len) {
  file_->append(logline, len);

  if (file_->writtenBytes() > roll_size_) {
    rollFile();
  } else {
    ++count_;
    if (count_ >= check_every_n_) {
      count_ = 0;
      auto now = ::time(nullptr);
      auto this_period = now / kRollPerSeconds_ * kRollPerSeconds_;
      if (this_period != start_) {
        rollFile();
      } else if (now - last_flush_ > flush_interval_) {
        last_flush_ = now;
        file_->flush();
      }
    }
  }
}

void LogFile::flush() { file_->flush(); }

void LogFile::rollFile() {
  auto filename = getLogFileName(basename_);
  file_.reset(new FileUtil(filename.c_str()));
}

std::string LogFile::getLogFileName(const std::string &basename) {
  std::string filename;
  filename.reserve(basename.size() + 32);
  filename = basename;

  char timebuf[32];
  struct tm tm;
  time_t now;
  now = time(NULL);
  gmtime_r(&now, &tm);
  strftime(timebuf, sizeof timebuf, ".%Y%m%d-%H%M%S.", &tm);
  // timebuf : .20181204-235959.
  filename += timebuf;

  filename += "log";

  return filename;
}

} // namespace bamboo