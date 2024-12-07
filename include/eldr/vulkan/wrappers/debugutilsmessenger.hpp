#pragma once
#include <eldr/vulkan/fwd.hpp>

#include <memory>
namespace eldr::vk::wr {
class DebugUtilsMessenger {
public:
  DebugUtilsMessenger(const Instance& instance);

private:
  class DebugUtilsMessengerImpl;
  std::shared_ptr<DebugUtilsMessengerImpl> messenger_data_;
};
} // namespace eldr::vk::wr
