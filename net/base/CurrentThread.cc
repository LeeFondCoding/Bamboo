#include "base/CurrentThread.h"

namespace bamboo {
namespace CurrentThread {

thread_local int t_cachedTid = 0;

void cacheTid() {
  if (t_cachedTid == 0) {
    t_cachedTid = static_cast<pid_t>(::syscall(SYS_gettid));
  }
}

} // namespace CurrentThread
} // namespace bamboo