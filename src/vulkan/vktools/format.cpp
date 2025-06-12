#include <eldr/vulkan/vulkan.hpp>

#include <vulkan/vk_enum_string_helper.h>

#define EL_VK_TYPE_TO_C_STR(type)                                              \
  template <> const char* eldr::vk::tools::toCharString<type>(const type& arg) \
  {                                                                            \
    return string_##type(arg);                                                 \
  }

#define EL_VK_FLAGS_TO_STR(type)                                               \
  template <> std::string eldr::vk::tools::toString<type>(const type& arg)     \
  {                                                                            \
    return string_##type(arg);                                                 \
  }

// Enum types
EL_VK_TYPE_TO_C_STR(VkResult)
EL_VK_TYPE_TO_C_STR(VkImageLayout)
EL_VK_TYPE_TO_C_STR(VkFormat)
EL_VK_TYPE_TO_C_STR(VkSampleCountFlagBits)
EL_VK_TYPE_TO_C_STR(VkAccessFlagBits2)

// Flags
EL_VK_FLAGS_TO_STR(VkSampleCountFlags)
// EL_VK_FLAGS_TO_STR(VkAccessFlags2)
