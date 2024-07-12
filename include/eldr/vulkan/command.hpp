#pragma once

#include <eldr/vulkan/commandpool.hpp>
#include <eldr/vulkan/common.hpp>

namespace eldr {
namespace vk {

class SingleTimeCommand {
public:
  SingleTimeCommand();
  SingleTimeCommand(const Device* device_, CommandPool* const);
  ~SingleTimeCommand();

  VkCommandBuffer& buffer() { return command_buffer_; }
  void             submit();

private:
  const Device*      device_;
  CommandPool* const command_pool_;

  VkCommandBuffer command_buffer_;
};
} // namespace vk
} // namespace eldr
