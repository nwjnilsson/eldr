#include <eldr/vulkan/vktools/format.hpp>

#include <vulkan/vk_enum_string_helper.h>
namespace std {
std::ostream& operator<<(std::ostream& os, const VkResult& result)
{
  os << string_VkResult(result);
  return os;
}
} // namespace std
