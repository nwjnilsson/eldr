#include <eldr/core/logger.hpp>

#include <spdlog/spdlog.h>

#include <unordered_map>

namespace eldr::core {
Logger requestLogger(const std::string& name)
{
  static std::unordered_map<std::string, Logger> loggers{};
  static std::mutex                              mutex;
  std::lock_guard<std::mutex>                    guard{ mutex };
  if (loggers.contains(name))
    return loggers[name];
  else {
    spdlog::trace("Creating new logger '{}'", name);
    loggers[name] = spdlog::default_logger()->clone(name);
    return loggers[name];
  }
}
} // namespace eldr::core
