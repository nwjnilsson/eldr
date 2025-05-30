#pragma once
#include <eldr/vulkan/fwd.hpp>

#include <memory>
namespace eldr::vk::wr {
class DebugUtilsMessenger {
public:
  DebugUtilsMessenger() = default;
  DebugUtilsMessenger(const Instance& instance);

private:
  class DebugUtilsMessengerImpl;
  std::shared_ptr<DebugUtilsMessengerImpl> d_;
};
} // namespace eldr::vk::wr
