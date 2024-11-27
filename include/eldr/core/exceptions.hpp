#pragma once

#include <stdexcept>

#define Throw(...)                                                             \
  do {                                                                         \
    throw std::runtime_error(fmt::format("[File: {}, line: {}] {}", __FILE__,  \
                                         __LINE__, fmt::format(__VA_ARGS__))); \
  } while (0)

#define ThrowVk(result, ...)                                                   \
  do {                                                                         \
    throw eldr::core::VulkanException(                                         \
      fmt::format("[File: {}, line: {}] {}", __FILE__, __LINE__,               \
                  fmt::format(__VA_ARGS__)),                                   \
      static_cast<detail::VulkanResult>(result));                              \
  } while (0)

namespace eldr::detail {
enum VulkanResult : int; // "forward" VkResult compatible type to avoid pulling
                         // in vulkan.h here
} // namespace eldr::detail

namespace eldr::core {
class VulkanException final : public std::runtime_error {
public:
  VulkanException(const std::string&         message,
                  eldr::detail::VulkanResult result);

private:
  static std::string createErrorMessage(std::string                message,
                                        eldr::detail::VulkanResult result);
};
} // namespace eldr::core
