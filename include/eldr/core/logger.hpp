#pragma once
#include <eldr/core/formatter.hpp>
#include <eldr/core/fwd.hpp>
#include <eldr/core/thread.hpp>
#include <eldr/eldr.hpp>

#include <fmt/format.h>

#include <memory>

NAMESPACE_BEGIN(eldr)

enum LogLevel : int {
  Trace,    /// Verbose logging
  Debug,    /// Debug message
  Info,     /// Information / relevant debug info
  Warn,     /// Warning message
  Error,    /// Error message
  Critical, /// Critical, causes an exception to be thrown
  n_levels
};

class Logger {
  /// PassKey class for `Logger`. `Thread`s can construct this object implicitly
  /// with "{}"
  class PassKey {
    friend class Thread;
    friend class Logger;
    PassKey() {}

  public:
    PassKey(const PassKey&) = default;
  };

public:
  /// Constructs a Logger, e.g "Logger({}, Warn);"
  explicit Logger(const PassKey&, LogLevel log_level = LogLevel::Debug);

  void log(LogLevel           level,
           const std::string& class_,
           const char*        function,
           const char*        file,
           int                line,
           const std::string& message);

  /// @brief Process a progress message
  /// @param formatted Formatted string representation of the message
  void logProgress(const std::string& formatted);

  /// Adds a sink to this logger.
  void addSink(std::shared_ptr<Sink> sink);

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
  [[nodiscard]] LogLevel logLevel() const { return log_level_; }

  /// Return the logger's error level
  [[nodiscard]] LogLevel errorLevel() const;

  /// Return the number of sinks for this logger
  [[nodiscard]] size_t sinkCount() const;

  /// Initialize logging
  static void createContext();

private:
  LogLevel log_level_;
  struct LoggerImpl;
  std::shared_ptr<LoggerImpl> impl_;
};

NAMESPACE_BEGIN(detail)

[[nodiscard]] std::string className(const char* function_sig);

[[noreturn]] void Throw(const std::string& class_,
                        const char*        function,
                        const char*        file,
                        int                line,
                        const std::string& message);

template <typename... Args>
static void Log(LogLevel                    level,
                const std::string&          class_,
                const char*                 function,
                const char*                 file,
                int                         line,
                fmt::format_string<Args...> fmt,
                Args&&... args)
{
  Logger* logger{ eldr::Thread::thread()->logger() };
  if (logger && level >= logger->logLevel()) {
    logger->log(level,
                class_,
                function,
                file,
                line,
                fmt::format(fmt, std::forward<Args>(args)...));
  }
}
NAMESPACE_END(detail)
NAMESPACE_END(eldr)

#if defined(__GNUC__) || defined(__clang__)
#  define EL_FUNCTION static_cast<const char*>(__PRETTY_FUNCTION__)
#elif defined(_MSC_VER)
#  define EL_FUNCTION static_cast<const char*>(__FUNCSIG__)
#else
// __func__ should always be available regardless of compiler but we can't
// extract class name from it
#  define EL_FUNCTION static_cast<const char*>(__func__)
#endif

#define EL_CLASS eldr::detail::className(EL_FUNCTION)

#define Log(level, ...)                                                        \
  do {                                                                         \
    eldr::detail::Log(                                                   \
      level, EL_CLASS, __func__, __FILE__, __LINE__, ##__VA_ARGS__);           \
  } while (0)

#define Throw(...)                                                             \
  do {                                                                         \
    eldr::detail::Throw(                                                 \
      EL_CLASS, __func__, __FILE__, __LINE__, fmt::format(__VA_ARGS__));       \
  } while (0)

#ifndef NDEBUG
/// Assert that a condition is true
#  define EL_ASSERT1(cond)                                                     \
    do {                                                                       \
      if (!(cond))                                                             \
        Throw("Assertion \"{}\" failed!", #cond);                              \
    } while (0)

/// Assertion with a specific error explanation
#  define EL_ASSERT2(cond, explanation)                                        \
    do {                                                                       \
      if (!(cond))                                                             \
        Throw("Assertion \"{}\" failed (" explanation ")", #cond);             \
    } while (0)

/// Expose both of the above macros using overloading, i.e.
/// <tt>Assert(cond)</tt> or <tt>Assert(cond, explanation)</tt>
#  define Assert(...)                                                          \
    EL_EXPAND(                                                                 \
      EL_EXPAND(EL_CONCAT(EL_ASSERT, EL_VA_SIZE(__VA_ARGS__)))(__VA_ARGS__))
#else
#  define Assert(...) ((void) 0)
#endif // !defined(NDEBUG)

/// Throw an exception reporting that the given function is not implemented
#define NotImplementedError Throw("%s: not implemented!", EL_FUNCTION);
