#include <eldr/core/logger.hpp>
#include <eldr/vulkan/wrappers/debugutilsmessenger.hpp>
#include <eldr/vulkan/wrappers/instance.hpp>

namespace eldr::vk::wr {

#ifdef ELDR_VULKAN_DEBUG_REPORT
static VKAPI_ATTR VkBool32 VKAPI_CALL vkDebugReportCallback(
  VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
  VkDebugUtilsMessageTypeFlagsEXT             messageType,
  const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
  (void) pUserData; // TODO: figure out how to use
  std::string type{};
  switch (messageType) {
    case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
      type = "general";
      break;
    case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
      type = "validation";
      break;
    case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
      type = "performance";
      break;
    case VK_DEBUG_UTILS_MESSAGE_TYPE_FLAG_BITS_MAX_ENUM_EXT:
      type = "flag bits max"; // what even
      break;
    default:
      type = "invalid";
      break;
  }

  auto logger = detail::requestLogger("vulkan-debug");
  switch (messageSeverity) {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
      logger->debug("(type = {}): {}", type, pCallbackData->pMessage);
      break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
      logger->info("(type = {}): {}", type, pCallbackData->pMessage);
      break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
      logger->warn("(type = {}): {}", type, pCallbackData->pMessage);
      break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
      logger->error("(type = {}): {}", type, pCallbackData->pMessage);
      break;
    default:
      break;
  }
  return VK_FALSE;
#endif
}

DebugUtilsMessenger::DebugUtilsMessenger(const Instance& instance)
  : instance_(instance)
{
#ifdef ELDR_VULKAN_DEBUG_REPORT
  VkDebugUtilsMessengerCreateInfoEXT debug_report_ci{};
  debug_report_ci.sType =
    VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  debug_report_ci.messageSeverity =
    VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
    VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
    VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  debug_report_ci.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  debug_report_ci.pfnUserCallback = vkDebugReportCallback;
  debug_report_ci.pUserData       = nullptr; // Optional

  auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(
    instance.get(), "vkCreateDebugUtilsMessengerEXT");
  assert(func != nullptr);
  VkResult err =
    func(instance.get(), &debug_report_ci, nullptr, &debug_messenger_);
  if (err != VK_SUCCESS)
    ThrowVk(err, "Failed to create debug utils messenger: ");
#endif
}

DebugUtilsMessenger::~DebugUtilsMessenger()
{
#ifdef ELDR_VULKAN_DEBUG_REPORT
  auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(
    instance_.get(), "vkDestroyDebugUtilsMessengerEXT");
  if (func != nullptr)
    func(instance_.get(), debug_messenger_, nullptr);
#endif
}
} // namespace eldr::vk::wr
