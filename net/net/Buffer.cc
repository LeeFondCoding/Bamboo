#include "net/Buffer.h"

#include "assert.h"
#include <sys/uio.h>
#include <unistd.h>

namespace bamboo {

const char Buffer::kCRLF[] = "\r\n";

Buffer::Buffer(size_t initial_size)
    : reader_index_(kCheapPrepend), writer_index_(kCheapPrepend),
      buffer_(kCheapPrepend + initial_size) {
  assert(readableBytes() == 0);
  assert(writeableBytes() == initial_size);
  assert(prependableBytes() == kCheapPrepend);
}

ssize_t Buffer::readFd(int fd, int *saveErrno) {
  char extrabuf[65536] = {0};
  struct iovec vec[2];
  const auto writable_bytes = writeableBytes();
  vec[0].iov_base = begin() + writer_index_;
  vec[0].iov_len = writable_bytes;
  vec[1].iov_base = extrabuf;
  vec[1].iov_len = sizeof(extrabuf);

  const int iov_cnt = writable_bytes < sizeof(extrabuf) ? 2 : 1;
  const auto n = ::readv(fd, vec, iov_cnt);

  if (n < 0) {
    *saveErrno = errno;
  } else if (n <= writable_bytes) {
    writer_index_ += n;
  } else {
    writer_index_ = buffer_.size();
    append(extrabuf, n - writable_bytes);
  }
  return n;
}

} // namespace bamboo