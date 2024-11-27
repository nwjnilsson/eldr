#include <eldr/core/exceptions.hpp>
#include <eldr/vulkan/vktools/format.hpp>

#include <fmt/format.h>
#include <vulkan/vulkan.h>

#include <sstream>
#include <stdexcept>

namespace eldr::core {
// -----------------------------------------------------------------------------
// VulkanException
// -----------------------------------------------------------------------------
VulkanException::VulkanException(const std::string&   message,
                                 detail::VulkanResult result)
  : std::runtime_error(createErrorMessage(message, result))
{
}

std::string VulkanException::createErrorMessage(std::string          message,
                                                detail::VulkanResult result)
{
  std::stringstream ss;
  if (static_cast<VkResult>(result) != VK_SUCCESS)
    ss << message << " [" << static_cast<VkResult>(result) << "]";
  else
    ss << message;
  return ss.str();
}
} // namespace eldr::core
