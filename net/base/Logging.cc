#include "base/Logging.h"

#include "base/CurrentThread.h"

#include "assert.h"

namespace {
// print message to stdout
void defaultOutput(const char *msg, size_t len) { fwrite(msg, 1, len, stdout); }

void defaultFlush() { fflush(stdout); }

bamboo::Logger::LogLevel initLogLevel() {
  if (::getenv("MUDUO_LOG_TRACE")) {
    return bamboo::Logger::TRACE;
  }
  if (::getenv("MUDUO_LOG_DEBUG")) {
    return bamboo::Logger::DEBUG;
  }
  return bamboo::Logger::INFO;
}

thread_local char t_errnobuf[512];
char t_time[64] = "not initialized";
time_t t_lastsecond = -1;

const char *LogLevelName[] = {
    "TRACE ", "DEBUG ", "INFO  ", "WARN  ", "ERROR ", "FATAL ",
};

} // namespace

namespace bamboo {

Logger::OutputFunc g_output = defaultOutput;
Logger::FlushFunc g_flush = defaultFlush;
Logger::LogLevel g_logLevel = initLogLevel();

Logger::LogLevel Logger::logLevel() { return g_logLevel; }

void setLogLevel(Logger::LogLevel level) { g_logLevel = level; }

void Logger::setOutput(OutputFunc out) { g_output = out; }

void Logger::setFlush(FlushFunc flush) { g_flush = flush; }

Logger::Logger(const char *file, int line) : impl_(INFO, 0, file, line) {}

Logger::Logger(const char *file, int line, LogLevel level)
    : impl_(level, 0, file, line) {}

Logger::Logger(const char *file, int line, LogLevel level, const char *func)
    : impl_(level, 0, file, line) {
  impl_.stream_ << func << ' ';
}

Logger::Logger(const char *file, int line, bool toAbort)
    : impl_(toAbort ? FATAL : ERROR, errno, file, line) {}

Logger::~Logger() {
  impl_.finish();
  const auto &buf(stream().buffer());
  g_output(buf.data(), buf.length());

  if (impl_.level_ == FATAL) {
    g_flush();
    abort();
  }
}

Logger::Impl::Impl(LogLevel level, int savedErrno, const char *file, int line)
    : time_(TimeStamp::now()), level_(level), line_(line), basename_(file) {
  formatTime();
  stream_ << " " << CurrentThread::tid();
  stream_ << " " << LogLevelName[level];
}

void Logger::Impl::formatTime() {
  int64_t microSecondsSinceEpoch = time_.microSecondsSinceEpoch();
  time_t seconds = static_cast<time_t>(microSecondsSinceEpoch /
                                       TimeStamp::kMicroSecondsPerSecond);
  int microseconds = static_cast<int>(microSecondsSinceEpoch %
                                      TimeStamp::kMicroSecondsPerSecond);
  if (seconds != t_lastsecond) {
    t_lastsecond = seconds;
    struct tm tm_time;
    ::gmtime_r(&microSecondsSinceEpoch, &tm_time);
    int len =
        snprintf(t_time, sizeof(t_time), "%4d-%02d-%02d %02d:%02d:%02d",
                 tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                 tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
    assert(len == 19);
  }
  stream_ << t_time << " " << microseconds;
}

void Logger::Impl::finish() {
  stream_ << " - " << basename_ << ':' << line_ << '\n';
}

// thread safe version of strerror
const char *strerror_tl(int savedErrno) {
  return strerror_r(savedErrno, t_errnobuf, sizeof t_errnobuf);
}

} // namespace bamboo