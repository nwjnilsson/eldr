#pragma once
#include <eldr/vulkan/fwd.hpp>

#include <memory>
namespace eldr::vk::wr {
class DebugUtilsMessenger {
public:
  DebugUtilsMessenger();
  DebugUtilsMessenger(const Instance& instance);
  ~DebugUtilsMessenger();

  DebugUtilsMessenger& operator=(DebugUtilsMessenger&&);

private:
  class DebugUtilsMessengerImpl;
  std::unique_ptr<DebugUtilsMessengerImpl> d_;
};
} // namespace eldr::vk::wr
