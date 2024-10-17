#include "FileUtil.h"
#include <cerrno>
#include <cstddef>
#include <cstdio>

namespace bamboo {
FileUtil::FileUtil(const char *filename)
    : fp_(::fopen(filename, "ae")) { // 'e' for O_CLOEXEC
  ::setbuffer(fp_, buffer_, sizeof(buffer_));
}

FileUtil::~FileUtil() { ::fclose(fp_); }
void FileUtil::append(const char *logline, size_t len) {
  auto has_written = write(logline, len);
  auto remain = len - has_written;

  while (remain > 0) {
    size_t x = write(logline + has_written, remain);
    if (x == 0) {
      int err = ferror(fp_);
      if (err)
        fprintf(stderr, "AppendFile::append() failed : %d", err);
      break;
    }
    has_written += x;
    remain = len - has_written;
  }
  written_bytes_ += len;
}

void FileUtil::flush() { ::fflush(fp_); }

size_t FileUtil::write(const char *logline, size_t len) {
  return ::fwrite_unlocked(logline, 1, len, fp_);
}

} // namespace bamboo