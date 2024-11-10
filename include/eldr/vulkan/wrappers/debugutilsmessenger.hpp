#pragma once

#include <eldr/vulkan/common.hpp>

namespace eldr::vk::wr {
class DebugUtilsMessenger {
public:
  DebugUtilsMessenger() = delete;
  DebugUtilsMessenger(const Instance& instance);
  ~DebugUtilsMessenger();

private:
  const Instance&          instance_;
  VkDebugUtilsMessengerEXT debug_messenger_;
};
} // namespace eldr::vk::wr
