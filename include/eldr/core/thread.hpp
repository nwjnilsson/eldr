#pragma once
#include <eldr/core/fwd.hpp>

#include <string>

namespace eldr::core {
class Thread {
public:
  [[nodiscard]] const std::string& name() const { return name_; }

  [[nodiscard]] Logger* logger();

  [[nodiscard]] static Thread* thread();

private:
  std::string name_;
};
} // namespace eldr::core
