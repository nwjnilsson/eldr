#include <vulkan/wrappers/debugutilsmessenger.hpp>
#include <vulkan/wrappers/instance.hpp>

#ifdef ELDR_VULKAN_DEBUG_REPORT

NAMESPACE_BEGIN(eldr::vk)

//------------------------------------------------------------------------------
// Debug report callback
//------------------------------------------------------------------------------
NAMESPACE_BEGIN()
VKAPI_ATTR VkBool32 VKAPI_CALL
vkDebugReportCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                      VkDebugUtilsMessageTypeFlagsEXT        messageType,
                      const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                      void*                                       pUserData)
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

  switch (messageSeverity) {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
      Log(Debug, "(type = {}): {}", type, pCallbackData->pMessage);
      break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
      Log(Info, "(type = {}): {}", type, pCallbackData->pMessage);
      break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
      Log(Warn, "(type = {}): {}", type, pCallbackData->pMessage);
      break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
      Log(Error, "(type = {}): {}", type, pCallbackData->pMessage);
      break;
    default:
      break;
  }
  return VK_FALSE;
}
NAMESPACE_END()

//------------------------------------------------------------------------------
// DebugUtilsMessenger
//------------------------------------------------------------------------------
DebugUtilsMessenger::DebugUtilsMessenger() = default;
DebugUtilsMessenger::DebugUtilsMessenger(DebugUtilsMessenger&&) noexcept =
  default;
DebugUtilsMessenger& DebugUtilsMessenger::operator=(DebugUtilsMessenger&& o)
{
  if (this != &o) {
    if (vk()) {
      auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(
        instance().vk(), "vkDestroyDebugUtilsMessengerEXT");
      if (func != nullptr)
        func(instance().vk(), object_, nullptr);
    }
    Base::operator=(std::move(o));
  }
  return *this;
}

DebugUtilsMessenger::DebugUtilsMessenger(std::string_view name,
                                         const Instance&  _instance)
  : Base(name, _instance)
{
  const VkDebugUtilsMessengerCreateInfoEXT debug_report_ci{
    .sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
    .pNext           = {},
    .flags           = {},
    .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
    .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                   VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                   VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
    .pfnUserCallback = vkDebugReportCallback,
    .pUserData       = {},
  };
  auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(
    instance().vk(), "vkCreateDebugUtilsMessengerEXT");
  Assert(func != nullptr);
  const VkResult result{ func(
    instance().vk(), &debug_report_ci, nullptr, &object_) };
  if (result != VK_SUCCESS)
    Throw("Failed to create debug utils messenger! ({})", result);
}

DebugUtilsMessenger::~DebugUtilsMessenger()
{
  if (vk()) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(
      instance().vk(), "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr)
      func(instance().vk(), vk(), nullptr);
  }
}
NAMESPACE_END(eldr::vk)
#endif
