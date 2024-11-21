#pragma once

#include "base/LogStream.h"
#include "base/TimeStamp.h"

#include <cstring>

namespace bamboo {

class Logger {
public:
  using OutputFunc = void (*)(const char *, size_t);
  using FlushFunc = void (*)();

  enum LogLevel { TRACE, DEBUG, INFO, WARN, ERROR, FATAL, NUM_LOG_LEVELS };

  static LogLevel logLevel();
  static void setLogLevel(LogLevel level);
  static void setOutput(OutputFunc);
  static void setFlush(FlushFunc);

  LogStream &stream() { return impl_.stream_; }
  Logger(const char *file, int line);
  Logger(const char *file, int line, LogLevel level);
  Logger(const char *file, int line, LogLevel level, const char *func);
  Logger(const char *file, int line, bool toAbort);
  ~Logger();

  void debugImplFormatTime() { impl_.formatTime(); }

private:
  class Impl {
  public:
    Impl(LogLevel level, int old_errno, const char *file, int line);
    void formatTime();
    void finish();

    TimeStamp time_;
    LogStream stream_;
    LogLevel level_;
    int line_;
    const char *basename_;
  };

  Impl impl_;
};

extern Logger::LogLevel g_logLevel;

inline Logger::LogLevel logLevel() { return g_logLevel; }

#define LOG_TRACE                                                              \
  if (Logger::logLevel() <= Logger::TRACE)                                     \
  Logger(__SHORT_FILE__, __LINE__, Logger::TRACE, __func__).stream()

#define LOG_DEBUG                                                              \
  if (Logger::logLevel() <= Logger::DEBUG)                                     \
  Logger(__SHORT_FILE__, __LINE__, Logger::DEBUG, __func__).stream()

#define LOG_INFO                                                               \
  if (Logger::logLevel() <= Logger::INFO)                                      \
  Logger(__SHORT_FILE__, __LINE__).stream()

#define LOG_WARN Logger(__SHORT_FILE__, __LINE__, Logger::WARN).stream()

#define LOG_ERROR Logger(__SHORT_FILE__, __LINE__, bamboo::Logger::ERROR).stream()

#define LOG_FATAL Logger(__SHORT_FILE__, __LINE__, Logger::FATAL).stream()

#define LOG_SYSERR Logger(__SHORT_FILE__, __LINE__, false).stream()

#define LOG_SYSFATAL Logger(__SHORT_FILE__, __LINE__, true).stream()

// /home/user/bamboo/net/base/Logging.h ==>> Logging.h
#define __SHORT_FILE__                                                         \
  (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

const char *strerror_tl(int saved_err);

} // namespace bamboo