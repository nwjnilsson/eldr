#include <sstream>

#include <eldr/vulkan/exceptions.hpp>
#include <eldr/vulkan/vktools/format.hpp>

namespace eldr::vk {
VulkanException::VulkanException(std::string_view     message,
                                 detail::VulkanResult result)
  : std::runtime_error(createErrorMessage(message, result))
{
}

std::string VulkanException::createErrorMessage(std::string_view     message,
                                                detail::VulkanResult result)
{
  std::stringstream ss;
  if (static_cast<VkResult>(result) != VK_SUCCESS)
    ss << message << " [" << static_cast<VkResult>(result) << "]";
  else
    ss << message;
  return ss.str();
}
} // namespace eldr::vk
