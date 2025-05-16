#include <eldr/core/formatter.hpp>
#include <eldr/core/logger.hpp>
#include <eldr/core/sink.hpp>

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#include <spdlog/pattern_formatter.h>
#include <spdlog/spdlog.h>

#include <iostream>

namespace eldr::core::logging {
struct Logger::LoggerImpl {
  std::mutex                      mutex;
  LogLevel                        error_level{ LogLevel::Error };
  std::unique_ptr<spdlog::logger> logger;
  std::unique_ptr<Formatter>      formatter;
};

Logger::Logger(const std::string& name, PassKey, LogLevel level)
  : log_level_(level), impl_(std::make_unique<LoggerImpl>())
{
  impl_->logger = std::make_unique<spdlog::logger>(name);
}

const Formatter* Logger::formatter() const { return impl_->formatter.get(); }

LogLevel Logger::errorLevel() const { return impl_->error_level; }

LogLevel Logger::logLevel() const
{
  return static_cast<LogLevel>(impl_->logger->level());
}

size_t Logger::sinkCount() const { return impl_->logger->sinks().size(); }

void Logger::setFormatter(std::unique_ptr<Formatter> formatter)
{
  // spdlog does not have a get_formatter() method, so I clone the formatter
  // here and save it in impl_ in case I want to get a pointer to the formatter
  // at some point to do some formatting
  auto clone = formatter->formatter_->clone();
  impl_->logger->set_formatter(std::move(clone));
  impl_->formatter = std::move(formatter);
}

void Logger::setLogLevel(LogLevel level)
{
  Assert(level <= impl_->error_level);
  impl_->logger->set_level(static_cast<spdlog::level::level_enum>(level));
}

void Logger::setErrorLevel(LogLevel level)
{
  Assert(level >= logLevel());
  impl_->error_level = level;
}

void Logger::addSink(std::shared_ptr<Sink> sink)
{
  impl_->logger->sinks().push_back(sink);
}

void Logger::clearSinks() { impl_->logger->sinks().clear(); }

#undef Throw
void Logger::log(LogLevel           level,
                 const char*        filename,
                 int                line,
                 const char*        function,
                 const std::string& message)
{
  if (level < logLevel()) {
    return;
  }
  else if (level >= impl_->error_level) {
    detail::Throw(level, filename, line, function, message);
  }
  if (!impl_->formatter) {
    std::cerr << "PANIC: Logging has not been properly initialized\n";
    abort();
  }

  std::lock_guard<std::mutex> guard{ impl_->mutex };
  impl_->logger->log(static_cast<spdlog::level::level_enum>(level), message);
}

namespace detail {
#undef Throw
void Throw(LogLevel level, const char* file, int line, const std::string& msg)
{
  auto formatter{ std::make_unique<spdlog::pattern_formatter>() };
  //  [time] [Logger name] [Thread name] [color+level+endcolor] message
  formatter->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%^%l%$] %v");

  formatter->format();
}
} // namespace detail

} // namespace eldr::core::logging
