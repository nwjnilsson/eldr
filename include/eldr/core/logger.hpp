#pragma once
#include <eldr/core/formatter.hpp>
#include <eldr/core/fwd.hpp>
#include <eldr/core/thread.hpp>

#include <fmt/format.h>
#include <spdlog/fwd.h>

#include <memory>

namespace eldr::core::logging {

enum LogLevel : int { // log level type compatible with spdlog
  Trace,
  Debug,
  Info,
  Warn,
  Error,
  Critical,
  Off,
  n_levels
};

class Logger {
  /// Only a `Thread` can create a `Logger`
  class PassKey {
    friend class Thread;
    PassKey() {}
  };

public:
  explicit Logger(const std::string& name,
                  PassKey,
                  LogLevel log_level = LogLevel::Debug);

  void log(LogLevel           level,
           const char*        filename,
           int                line,
           const char*        function,
           const std::string& message);

  /// Adds a sink to this logger.
  void addSink(std::shared_ptr<spdlog::sinks::sink> sink);

  /// Clears all sinks from this logger.
  void clearSinks();

  /// Sets the formatter for this logger.
  void setFormatter(std::unique_ptr<Formatter> formatter);

  /// Sets the log level of this logger.
  /// @param level Must be lower or equal the current error level.
  void setLogLevel(LogLevel level);

  /// Sets the error level of this logger.
  /// @param level Must be higher or equal to the log level.
  void setErrorLevel(LogLevel level);

  /// Return the logger's formatter
  [[nodiscard]] const Formatter* formatter() const;

  /// Return the logger's log level
  [[nodiscard]] LogLevel logLevel() const;

  /// Return the logger's error level
  [[nodiscard]] LogLevel errorLevel() const;

  /// Return the number of sinks for this logger
  [[nodiscard]] size_t sinkCount() const;

private:
  // LogLevel log_level_;
  struct LoggerImpl;
  std::shared_ptr<LoggerImpl> impl_;
};

namespace detail {

[[noreturn]] void Throw(LogLevel           level,
                        const char*        file,
                        int                line,
                        const char*        function,
                        const std::string& msg);

template <typename... Args>
static void Log(LogLevel    level,
                const char* filename,
                int         line,
                const char* function,
                Args&&... args)
{
  Logger* logger{ eldr::core::Thread::thread()->logger() };
  if (logger && level >= logger->logLevel()) {
    logger->log(level,
                filename,
                line,
                function,
                fmt::format(std::forward<Args>(args)...));
  }
}
} // namespace detail
} // namespace eldr::core::logging

#define EL_FUNCTION static_cast<const char*>(__FUNCTION__)
#define Log(level, ...)                                                        \
  do {                                                                         \
    eldr::core::logging::detail::Log(                                          \
      level, __FILE__, __LINE__, EL_FUNCTION, __VA_ARGS__);                    \
  } while (0)

#define Throw(...)                                                             \
  do {                                                                         \
    eldr::core::logging::detail::Throw(                                        \
      Error, __FILE__, __LINE__, EL_FUNCTION, fmt::format(__VA_ARGS__));       \
  } while (0)

#ifndef NDEBUG
/// Assert that a condition is true
#  define EL_ASSERT1(cond)                                                     \
    do {                                                                       \
      if (!(cond))                                                             \
        Throw("Assertion \"%s\" failed!", #cond);                              \
    } while (0)

/// Assertion with a specific error explanation
#  define EL_ASSERT2(cond, explanation)                                        \
    do {                                                                       \
      if (!(cond))                                                             \
        Throw("Assertion \"%s\" failed (" explanation ")", #cond);             \
    } while (0)

/// Expose both of the above macros using overloading, i.e.
/// <tt>Assert(cond)</tt> or <tt>Assert(cond, explanation)</tt>
#  define Assert(...)                                                          \
    EL_EXPAND(                                                                 \
      EL_EXPAND(EL_CAT(EL_ASSERT, EL_VA_SIZE(__VA_ARGS__)))(__VA_ARGS__))
#else
#  define Assert(...) ((void) 0)
#endif // !defined(NDEBUG)

/// Throw an exception reporting that the given function is not implemented
#define NotImplementedError(Name)                                              \
  Throw("%s::" Name "(): not implemented!", class_()->name());
