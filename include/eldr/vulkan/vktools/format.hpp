#pragma once
#include <vulkan/vulkan_core.h>

#include <fmt/core.h>

namespace eldr::vk::tools {
template <typename T> const char* toCharString(const T& arg);
template <typename T> std::string toString(const T& arg);
} // namespace eldr::vk::tools

#define EL_IMPL_VK_FMT(type, str_func)                                         \
  template <> struct fmt::formatter<type> {                                    \
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }    \
    template <typename FormatContext>                                          \
    constexpr auto format(const type& arg, FormatContext& ctx) const           \
    {                                                                          \
      return format_to(ctx.out(), "{}", eldr::vk::tools::str_func<type>(arg)); \
    }                                                                          \
  };

#define EL_IMPL_VK_TYPE_FMT(type) EL_IMPL_VK_FMT(type, toCharString)
#define EL_IMPL_VK_FLAGS_FMT(type) EL_IMPL_VK_FMT(type, toString)

// Regular enum types
EL_IMPL_VK_TYPE_FMT(VkResult)
EL_IMPL_VK_TYPE_FMT(VkImageLayout)
EL_IMPL_VK_TYPE_FMT(VkFormat)
EL_IMPL_VK_TYPE_FMT(VkSampleCountFlagBits)
EL_IMPL_VK_TYPE_FMT(VkAccessFlagBits2)

// Flags
EL_IMPL_VK_FLAGS_FMT(VkSampleCountFlags)
// EL_IMPL_VK_FLAGS_FMT(VkAccessFlags2)

#undef EL_IMPL_VK_TYPE_FMT
#undef EL_IMPL_VK_FLAGS_FMT
#undef EL_IMPL_VK_FMT
