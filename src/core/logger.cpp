#include <eldr/core/formatter.hpp>
#include <eldr/core/logger.hpp>
#include <eldr/core/sink.hpp>

#include <iostream>
#include <memory>
#include <vector>

namespace eldr::core {
struct Logger::LoggerImpl {
  LogLevel                   error_level{ LogLevel::Error };
  std::unique_ptr<Formatter> formatter;
  std::vector<std::shared_ptr<Sink<ThreadingPolicy>>> sinks;
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

template <typename SinkType>
void Logger::addSink(std::shared_ptr<SinkType> sink)
{
  impl_->sinks.push_back(
    std::dynamic_pointer_cast<Sink<ThreadingPolicy>>(sink));
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
    detail::Throw(level, class_, function, file, line, message);
  }
  if (!impl_->formatter) {
    std::cerr << "PANIC: Logging has not been properly initialized\n";
    abort();
  }
  std::string text{ impl_->formatter->format(
    level, Thread::thread(), class_, function, file, line, message) };

  for (auto& sink : impl_->sinks) {
    (*sink)(level, message);
  }
}

void Logger::createContext()
{
  auto logger    = std::make_unique<Logger>(PassKey{}, Info);
  auto sink      = std::make_shared<StreamSink<MultiThreaded>>(&std::cout);
  auto formatter = std::make_unique<DefaultFormatter>();

  logger->setFormatter(std::move(formatter));
  logger->addSink(sink);

  Thread::thread()->setLogger(std::move(logger));
#ifdef DEBUG
  logger->setLogLevel(Debug);
#endif
}

namespace detail {

/// Extract the class name from a function signature
std::string className(const char* function_sig)
{
  /// function sig is either a full signature like "int Class::name(A param)" or
  /// simply "name" for compilers that don't support __PRETTY_FUNCTION__-like
  /// variables
  std::string  func_sig{ function_sig };
  std::string  class_name;
  const size_t colons{ func_sig.find("::") };
  if (colons == std::string::npos)
    return "Unknown";
  const size_t begin{ func_sig.substr(0, colons).rfind(" ") + 1 };
  const size_t end{ colons - begin };
  return func_sig.substr(begin, end);
}

#undef Throw

void Throw(LogLevel           level,
           const std::string& class_,
           const char*        function,
           const char*        file,
           int                line,
           const std::string& message)
{

  DefaultFormatter formatter;
  formatter.setHasFile(true);
  formatter.setClassFuncFormat(DefaultFormatter::ClassFuncFormat::ClassAndFunc);

  // Tag beginning of exception text with UTF8 zero width space
  const std::string zerowidth_space = "\xe2\x80\x8b";

  // Separate nested exceptions by a newline
  std::string msg{ message };
  size_t      pos{ msg.find(zerowidth_space) };
  if (pos != std::string::npos)
    msg = msg.substr(0, pos) + "\n  " + msg.substr(pos + 3);

  std::string text = formatter.format(
    level, Thread::thread(), class_, function, file, line, msg);
  throw std::runtime_error(zerowidth_space + text);
}

} // namespace detail

} // namespace eldr::core
