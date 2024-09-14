#include <eldr/core/logger.hpp>
#include <eldr/vulkan/helpers.hpp>

namespace eldr::vk::wr {
bool hasStencilComponent(VkFormat format)
{
  return format == VK_FORMAT_D32_SFLOAT_S8_UINT ||
         format == VK_FORMAT_D24_UNORM_S8_UINT;
}
} // namespace eldr::vk::wr
