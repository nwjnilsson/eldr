#include <eldr/core/logger.hpp>

namespace eldr::detail {
std::shared_ptr<spdlog::logger> requestLogger(const std::string& name)
{
  static std::unordered_map<std::string, std::shared_ptr<spdlog::logger>>
                              loggers{};
  static std::mutex           mutex;
  std::lock_guard<std::mutex> guard(mutex);
  if (loggers.contains(name))
    return loggers[name];
  else {
    loggers[name] = spdlog::default_logger()->clone(name);
    return loggers[name];
  }
}
} // namespace eldr::detail
