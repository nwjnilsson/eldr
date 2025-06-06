#include <eldr/core/formatter.hpp>
#include <eldr/core/logger.hpp>
#include <eldr/core/sink.hpp>

#include <iostream>
#include <memory>
#include <vector>

namespace eldr::core {
struct Logger::LoggerImpl {
  LogLevel                           error_level{ LogLevel::Error };
  std::unique_ptr<Formatter>         formatter;
  std::vector<std::shared_ptr<Sink>> sinks;
};

Logger::Logger(const PassKey&, LogLevel level)
  : log_level_(level), impl_(std::make_shared<LoggerImpl>())
{
}

const Formatter* Logger::formatter() const { return impl_->formatter.get(); }

LogLevel Logger::errorLevel() const { return impl_->error_level; }

size_t Logger::sinkCount() const { return impl_->sinks.size(); }

void Logger::setFormatter(std::unique_ptr<Formatter> formatter)
{
  impl_->formatter = std::move(formatter);
}

void Logger::setLogLevel(LogLevel level)
{
  Assert(level <= impl_->error_level);
  log_level_ = level;
}

void Logger::setErrorLevel(LogLevel level)
{
  Assert(level >= logLevel());
  impl_->error_level = level;
}

void Logger::logProgress(const std::string& formatted)
{
  for (auto& entry : impl_->sinks)
    entry->logProgress(formatted);
}

void Logger::addSink(std::shared_ptr<Sink> sink)
{
  impl_->sinks.push_back(std::move(sink));
}

void Logger::clearSinks() { impl_->sinks.clear(); }

#undef Throw
void Logger::log(LogLevel           level,
                 const std::string& class_,
                 const char*        function,
                 const char*        file,
                 int                line,
                 const std::string& message)
{
  if (level < logLevel()) {
    return;
  }
  else if (level >= impl_->error_level) {
    detail::Throw(class_, function, file, line, message);
  }
  if (!impl_->formatter) {
    std::cerr << "PANIC: Logging has not been properly initialized\n";
    abort();
  }
  const std::string text{ impl_->formatter->format(
    Thread::thread(), class_, function, file, line, message) };

  for (auto& sink : impl_->sinks) {
    (*sink)(level, text);
  }
}

void Logger::createContext()
{
  auto logger = std::make_unique<Logger>(PassKey{}, Info);
  // TODO: Multithreaded policy isn't bulletproof since different sink objects
  // with the same destination can be created. Each object then gets its own
  // lock. Would need a sink registry to keep track of destinations. What do?
  auto sink      = std::make_shared<StreamSink<MultiThreaded>>(&std::cout);
  auto formatter = std::make_unique<DefaultFormatter>();
  formatter->setHasDate(false);

  logger->setFormatter(std::move(formatter));
  logger->addSink(std::move(sink));
#if defined(LOG_ACTIVE_LEVEL_TRACE)
  logger->setLogLevel(Trace);
#elif defined(LOG_ACTIVE_LEVEL_DEBUG)
  logger->setLogLevel(Debug);
#elif defined(LOG_ACTIVE_LEVEL_INFO)
  logger->setLogLevel(Info);
#elif defined(LOG_ACTIVE_LEVEL_WARN)
  logger->setLogLevel(Warn);
#elif defined(LOG_ACTIVE_LEVEL_ERROR)
  logger->setLogLevel(Error);
#else
#  error "Default log level is not defined."
#endif
  Thread::thread()->setLogger(std::move(logger));
}

namespace detail {

/// Extract the class name from a function signature
std::string className(const char* function_sig)
{
  /// function sig is either a full signature like
  /// "void namespace::Class::function(A param)" or simply "function" for
  /// compilers that don't support __PRETTY_FUNCTION__-like variables
  std::string  func_sig{ function_sig };
  std::string  class_name;
  const size_t param_start{ func_sig.find("(") };
  assert(param_start != std::string::npos);
  const size_t r_colons{ func_sig.rfind("::", param_start) };
  if (r_colons == std::string::npos)
    return "Unknown";
  const size_t l_colons{ func_sig.rfind("::", r_colons - 1) };
  if (l_colons == std::string::npos) {
    const size_t space{ func_sig.rfind(" ", r_colons) };
    if (space == std::string::npos) {
      return "Unknown";
    }
    else {
      return func_sig.substr(space + 1, r_colons - (space + 1));
    }
  }
  else {
    return func_sig.substr(l_colons + 2, r_colons - (l_colons + 2));
  }
}

#undef Throw

void Throw(const std::string& class_,
           const char*        function,
           const char*        file,
           int                line,
           const std::string& message)
{

  DefaultFormatter formatter;
  formatter.setHasDate(false);
  formatter.setHasFile(false);
  formatter.setHasThread(false);
  formatter.setHasLogLevel(false);
  formatter.setClassFuncFormat(DefaultFormatter::ClassFuncFormat::ClassAndFunc);

  // Tag beginning of exception text with UTF8 zero width space
  const std::string zerowidth_space = "\xe2\x80\x8b";

  // Separate nested exceptions by a newline
  std::string msg{ message };
  size_t      pos{ msg.find(zerowidth_space) };
  if (pos != std::string::npos)
    msg = msg.substr(0, pos) + "\n  " + msg.substr(pos + 3);

  const std::string text{ formatter.format(
    Thread::thread(), class_, function, file, line, msg) };
  throw std::runtime_error(zerowidth_space + text);
}

} // namespace detail

} // namespace eldr::core
