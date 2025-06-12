#pragma once
#include <eldr/vulkan/fwd.hpp>

#include <memory>
NAMESPACE_BEGIN(eldr::vk::wr)
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
NAMESPACE_END(eldr::vk::wr)
