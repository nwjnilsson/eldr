#pragma once
#include <eldr/vulkan/vktools/format.hpp>

#include <sstream>
#include <stdexcept>

namespace eldr::vk {
class VulkanException final : public std::runtime_error {
public:
  VulkanException(const std::string& message)
    : std::runtime_error(createErrorMessage(message, VK_SUCCESS))
  {
  }

  VulkanException(const std::string& message, VkResult result)
    : std::runtime_error(createErrorMessage(message, result))
  {
  }

  // VkResult getErrorCode() const { return error_code_; }

private:
  // VkResult error_code_;

  static std::string createErrorMessage(std::string message, VkResult result)
  {
    std::stringstream ss;
    if (result != VK_SUCCESS)
      ss << message << " (" << result << ")";
    else
      ss << message;
    return ss.str();
  }
};
} // namespace eldr::vk
