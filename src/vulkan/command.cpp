#include <eldr/vulkan/command.hpp>

namespace eldr {
namespace vk {

SingleTimeCommand::SingleTimeCommand()
  : device_(nullptr), command_pool_(nullptr), command_buffer_(VK_NULL_HANDLE)
{
}
SingleTimeCommand::SingleTimeCommand(const Device*      device,
                                     CommandPool* const command_pool)
  : device_(device), command_pool_(command_pool)
{
  VkCommandBufferAllocateInfo alloc_info{};
  alloc_info.sType       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  alloc_info.commandPool = command_pool_->get();
  alloc_info.level       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  alloc_info.commandBufferCount = 1;

  vkAllocateCommandBuffers(device_->logical(), &alloc_info, &command_buffer_);

  VkCommandBufferBeginInfo begin_info{};
  begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  vkBeginCommandBuffer(command_buffer_, &begin_info);
}
SingleTimeCommand::~SingleTimeCommand()
{
  if (command_buffer_ != VK_NULL_HANDLE)
    vkFreeCommandBuffers(device_->logical(), command_pool_->get(), 1,
                         &command_buffer_);
}

// TODO: may want to optimize this function if used often (use fences instead of
// vkQueueWaitIdle)
void SingleTimeCommand::submit()
{
  vkEndCommandBuffer(command_buffer_);

  VkSubmitInfo submit_info{};
  submit_info.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers    = &command_buffer_;

  vkQueueSubmit(device_->graphicsQueue(), 1, &submit_info, VK_NULL_HANDLE);
  vkQueueWaitIdle(device_->graphicsQueue());
}
} // namespace vk
} // namespace eldr
