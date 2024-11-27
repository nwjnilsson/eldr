#pragma once
#include <spdlog/fwd.h>

#include <memory>

namespace eldr::core {
using Logger = std::shared_ptr<spdlog::logger>;
Logger requestLogger(const std::string& name);
} // namespace eldr::core
