#pragma once
#include <eldr/vulkan/common.hpp>

namespace eldr::vk::wr {
class CommandPool {
public:
  CommandPool();
  CommandPool(Device&, Surface&);
  ~CommandPool();

  VkCommandPool get() const { return pool_; }

private:
  Device& device_;

  VkCommandPool pool_;
};
} // namespace eldr::vk::wr
