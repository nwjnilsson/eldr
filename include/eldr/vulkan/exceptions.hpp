#include <stdexcept>
#include <string_view>

#define ThrowVk(result, ...)                                                   \
  do {                                                                         \
    throw eldr::vk::VulkanException(                                           \
      fmt::format("[File: {}, line: {}] {}",                                   \
                  __FILE__,                                                    \
                  __LINE__,                                                    \
                  fmt::format(__VA_ARGS__)),                                   \
      static_cast<detail::VulkanResult>(result));                              \
  } while (0)

// -----------------------------------------------------------------------------
// VulkanException
// -----------------------------------------------------------------------------
namespace eldr::detail {
enum VulkanResult : int; // "forward" VkResult compatible type to avoid pulling
                         // in vulkan.h here
} // namespace eldr::detail

namespace eldr::vk {
class VulkanException final : public std::runtime_error {
public:
  VulkanException(std::string_view message, detail::VulkanResult result);

private:
  static std::string createErrorMessage(std::string_view           message,
                                        eldr::detail::VulkanResult result);
};
} // namespace eldr::vk
