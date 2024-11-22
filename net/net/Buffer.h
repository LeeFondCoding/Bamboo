#pragma once

#include <algorithm>
#include <cstddef>
#include <string>
#include <vector>

namespace bamboo {

/// A buffer class modeled after org.jboss.netty.buffer.ChannelBuffer
///
/// @code
/// +-------------------+------------------+------------------+
/// | prependable bytes |  readable bytes  |  writable bytes  |
/// |                   |     (CONTENT)    |                  |
/// +-------------------+------------------+------------------+
/// |                   |                  |                  |
/// 0      <=      readerIndex   <=   writerIndex    <=     size
/// @endcode
class Buffer {
public:
  static constexpr size_t kCheapPrepend = 8;
  static constexpr size_t kInitialSize = 1024;

  explicit Buffer(size_t initialSize = kInitialSize)
      : reader_index_(kCheapPrepend), writer_index_(kCheapPrepend),
        buffer_(kCheapPrepend + initialSize) {}

  size_t readableBytes() const { return writer_index_ - reader_index_; }

  size_t writeableBytes() const { return buffer_.size() - writer_index_; }
  size_t prependableBytes() const { return reader_index_; }
  const char *peek() const { return begin() + reader_index_; }

  const char *findCRLF() const {
    const char *crlf = std::search(peek(), beginWrite(), kCRLF, kCRLF + 2);
    return crlf == beginWrite() ? nullptr : crlf;
  }

  void retrieve(size_t len) {
    if (len < readableBytes()) {
      reader_index_ += len;
    } else {
      retrieveAll();
    }
  }

  void retrieveUntil(const char *end) { retrieve(end - peek()); }

  void retrieveAll() { reader_index_ = writer_index_ = kCheapPrepend; }

  std::string retrieveAllString() { return retrieveAsString(readableBytes()); }

  std::string retrieveAsString(size_t len) {
    std::string res(peek(), len);
    retrieve(len);
    return res;
  }

  void ensureWriteable(size_t len) {
    if (writeableBytes() < len) {
      makeSpace(len);
    }
  }

  void append(const char *data, size_t len) {
    ensureWriteable(len);
    std::copy(data, data + len, begin() + writer_index_);
    writer_index_ += len;
  }

  void append(const std::string &str) { append(str.data(), str.size()); }

  char *beginWrite() {
    return begin() + writer_index_;
  }

  const char *beginWrite() const { return begin() + writer_index_; }

  ssize_t readFd(int fd, int *saveErrno);

private:
  char *begin() { return &*buffer_.begin(); }

  const char *begin() const { return &*buffer_.begin(); }

  void makeSpace(std::size_t len) {
    if (writeableBytes() < len) {
      buffer_.resize(writer_index_ + len + 1);
    }
  }

  size_t reader_index_;
  size_t writer_index_;
  std::vector<char> buffer_;

  static const char kCRLF[];
};
} // namespace bamboo