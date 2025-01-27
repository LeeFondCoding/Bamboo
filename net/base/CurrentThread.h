#pragma once

#include <sys/syscall.h>
#include <unistd.h>

namespace bamboo {
namespace CurrentThread {

extern thread_local int t_cachedTid;

void cacheTid();

// return current thread id
inline int tid() {
  if (__builtin_expect(t_cachedTid == 0, 0)) {
    cacheTid();
  }
  return t_cachedTid;
}

} // namespace CurrentThread
} // namespace bamboo
