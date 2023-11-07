#include <core/util.hpp>
#include <gui/vulkan-backend.hpp>
#include <set>
#include <spdlog/spdlog.h>
#include <stdexcept>

#include <gui/gui.hpp>
#include <vulkan/vulkan_core.h>

#define VK_EXT_DEBUG_UTILS_NAME "VK_EXT_debug_utils"

namespace eldr {
namespace vk_wrapper {

void throwVkErr(std::string msg)
{
  throw std::runtime_error("[VULKAN]: " + msg);
}

void checkVkResult(VkResult result)
{
  if (result == 0)
    return;
  spdlog::error("[VULKAN] Error: VkResult = %d", result);
  if (result < 0)
    throwVkErr("Fatal error!");
}

bool isAvailableVkExtension(const std::vector<VkExtensionProperties>& props,
                            const char*                               extension)
{
  for (const VkExtensionProperties& p : props)
    if (strcmp(p.extensionName, extension) == 0)
      return true;
  return false;
}

bool checkValidationLayerSupport(
  const std::vector<const char*> validation_layers)
{
  uint32_t layer_count;
  vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

  std::vector<VkLayerProperties> available_layers(layer_count);
  vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

  for (const char* layer_name : validation_layers) {
    bool layer_found = false;

    for (const auto& layer_properties : available_layers) {
      if (strcmp(layer_name, layer_properties.layerName) == 0) {
        layer_found = true;
        break;
      }
    }

    if (!layer_found) {
      return false;
    }
  }

  return true;
}

void VkWrapper::createInstance(std::vector<const char*>& instance_extensions)
{
  VkApplicationInfo app_info{};
  app_info.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.pApplicationName   = "Eldr";
  app_info.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
  app_info.pEngineName        = "No Engine";
  app_info.engineVersion      = VK_MAKE_VERSION(0, 0, 1);
  app_info.apiVersion         = VK_API_VERSION_1_2;

  VkResult err;
  {
    // Create Vulkan instance
    VkInstanceCreateInfo create_info = {};
    create_info.sType                = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo     = &app_info;

    uint32_t                           properties_count;
    std::vector<VkExtensionProperties> properties;
    vkEnumerateInstanceExtensionProperties(nullptr, &properties_count, nullptr);
    properties.resize(properties_count);
    err = vkEnumerateInstanceExtensionProperties(nullptr, &properties_count,
                                                 properties.data());
    checkVkResult(err);

    if (isAvailableVkExtension(
          properties, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME))
      instance_extensions.push_back(
        VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

#ifdef VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME
    if (isAvailableVkExtension(properties,
                               VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME)) {
      instance_extensions.push_back(
        VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
      create_info.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    }
#endif
    // Enabling validation layers
#ifdef ELDR_VULKAN_DEBUG_REPORT
    const std::vector<const char*> validation_layers = {
      "VK_LAYER_KHRONOS_validation"
    };
    if (!checkValidationLayerSupport(validation_layers)) {
      throwVkErr("Validation layers requested, but not available!");
    }
    create_info.enabledLayerCount =
      static_cast<uint32_t>(validation_layers.size());
    create_info.ppEnabledLayerNames = validation_layers.data();
    instance_extensions.push_back(VK_EXT_DEBUG_UTILS_NAME);
#endif

    create_info.enabledExtensionCount =
      static_cast<uint32_t>(instance_extensions.size());
    create_info.ppEnabledExtensionNames = instance_extensions.data();

    err = vkCreateInstance(&create_info, allocator_, &instance_);
    checkVkResult(err);
  }
}

#ifdef ELDR_VULKAN_DEBUG_REPORT
static VKAPI_ATTR VkBool32 VKAPI_CALL vkDebugReportCallback(
  VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
  VkDebugUtilsMessageTypeFlagsEXT             messageType,
  const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
  switch (messageSeverity) {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
      spdlog::trace("[VULKAN]: {}", pCallbackData->pMessage);
      break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
      spdlog::info("[VULKAN]: {}", pCallbackData->pMessage);
      break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
      spdlog::warn("[VULKAN]: {}", pCallbackData->pMessage);
      break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
      spdlog::error("[VULKAN]: {}", pCallbackData->pMessage);
      break;
    default:
      break;
  }

  return VK_FALSE;
}

void VkWrapper::setupDebugMessenger()
{
  // Debug report callback
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
    instance_, "vkCreateDebugUtilsMessengerEXT");
  EASSERT(func != nullptr);
  VkResult err =
    func(instance_, &debug_report_ci, allocator_, &debug_messenger_);
  checkVkResult(err);
}
#endif

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device,
                                     VkSurfaceKHR     surface)
{
  QueueFamilyIndices indices;
  uint32_t           count = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);
  std::vector<VkQueueFamilyProperties> queue_families(count);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &count,
                                           queue_families.data());

  // For now, only the graphics queue is found and set. Perhaps there will
  // be a need for other queues (memory or others) in the future?
  // Note on queue families: it is "very" likely that the graphics queue
  // family will be the same as the present queue family
  for (uint32_t i = 0; i < count; i++) {
    if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      indices.graphics_family = i;
    }
    VkBool32 present_support = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present_support);
    if (present_support)
      indices.present_family = i;
  }
  return indices;
}

SwapChainSupportDetails swapChainSupportDetails(VkPhysicalDevice device,
                                                VkSurfaceKHR     surface)
{
  SwapChainSupportDetails details;
  // Query for supported formats
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface,
                                            &details.capabilities);
  uint32_t format_count;
  vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, nullptr);

  if (format_count != 0) {
    details.formats.resize(format_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count,
                                         details.formats.data());
  }

  // Query for present modes
  uint32_t present_mode_count;
  vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface,
                                            &present_mode_count, nullptr);

  if (present_mode_count != 0) {
    details.present_modes.resize(present_mode_count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(
      device, surface, &present_mode_count, details.present_modes.data());
  }

  return details;
}

bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface,
                      std::vector<const char*> device_extensions)
{
  // Enumerate physical device extension
  uint32_t                           props_count;
  std::vector<VkExtensionProperties> properties;
  vkEnumerateDeviceExtensionProperties(device, nullptr, &props_count, nullptr);
  properties.resize(props_count);
  vkEnumerateDeviceExtensionProperties(device, nullptr, &props_count,
                                       properties.data());

  // Check extensions
  std::set<std::string> required_extensions(device_extensions.begin(),
                                            device_extensions.end());
  for (const auto& extension : properties) {
    required_extensions.erase(extension.extensionName);
  }
  // Queue families
  QueueFamilyIndices indices = findQueueFamilies(device, surface);
  // Swap chain
  SwapChainSupportDetails swap_chain_support =
    swapChainSupportDetails(device, surface);

  return required_extensions.empty() && indices.isComplete() &&
         !swap_chain_support.formats.empty() &&
         !swap_chain_support.present_modes.empty();
}

void VkWrapper::selectPhysicalDevice(
  std::vector<const char*>& device_extensions)
{
  uint32_t gpu_count;
  VkResult err = vkEnumeratePhysicalDevices(instance_, &gpu_count, nullptr);
  checkVkResult(err);

  if (gpu_count == 0)
    throwVkErr("No compatible device found");

  std::vector<VkPhysicalDevice> gpus;
  gpus.resize(gpu_count);
  err = vkEnumeratePhysicalDevices(instance_, &gpu_count, gpus.data());
  checkVkResult(err);

  // Find discrete GPU if available
  // It is possible to add additional criteria for what is to be considered
  // a suitable GPU. The Vulkan programming tutorial suggests implementing a
  // scoring system where each usable property of a device would add to its
  // overall score, and when the properties of all devices have been evaluated,
  // the highest scoring device is considered the most suitable one.
  // For now I don't really think it matters. Just use a discrete GPU if it's
  // available.
  for (VkPhysicalDevice& device : gpus) {
    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(device, &props);
    if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
        isDeviceSuitable(device, surface_, device_extensions)) {
      physical_device_ = device;
      return;
    }
  }
  // Return integrated GPU if no discrete one is present
  physical_device_ = gpus[0];
}

void VkWrapper::createLogicalDevice()
{

  std::vector<const char*> device_extensions;
  device_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
#ifdef VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME
  device_extensions.push_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
#endif

  selectPhysicalDevice(device_extensions);

  QueueFamilyIndices indices = findQueueFamilies(physical_device_, surface_);
  std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
  std::set<uint32_t> unique_queue_families = { indices.graphics_family.value(),
                                               indices.present_family.value() };

  float queue_priority = 1.0f;
  for (uint32_t queue_family : unique_queue_families) {
    VkDeviceQueueCreateInfo queue_create_info{};
    queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info.queueFamilyIndex = queue_family;
    queue_create_info.queueCount       = 1;
    queue_create_info.pQueuePriorities = &queue_priority;
    queue_create_infos.push_back(queue_create_info);
  }

  // Logical device creation
  VkDeviceCreateInfo create_info = {};
  create_info.sType              = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  create_info.queueCreateInfoCount =
    static_cast<uint32_t>(queue_create_infos.size());
  create_info.pQueueCreateInfos = queue_create_infos.data();
  create_info.enabledExtensionCount =
    static_cast<uint32_t>(device_extensions.size());
  create_info.ppEnabledExtensionNames = device_extensions.data();
  VkResult err =
    vkCreateDevice(physical_device_, &create_info, allocator_, &device_);
  checkVkResult(err);

  vkGetDeviceQueue(device_, indices.present_family.value(), 0, &present_queue_);
  vkGetDeviceQueue(device_, indices.graphics_family.value(), 0,
                   &graphics_queue_);
}

void VkWrapper::createCommandPool()
{
  VkDescriptorPoolSize pool_sizes[] = {
    { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 },
  };
  VkDescriptorPoolCreateInfo pool_info = {};
  pool_info.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  pool_info.flags         = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
  pool_info.maxSets       = 1;
  pool_info.poolSizeCount = (uint32_t) IM_ARRAYSIZE(pool_sizes);
  pool_info.pPoolSizes    = pool_sizes;
  VkResult err =
    vkCreateDescriptorPool(device_, &pool_info, allocator_, &descriptor_pool_);
  checkVkResult(err);
}

void VkWrapper::init(VkWrapperInitInfo& init_info)
{
  createInstance(init_info.instance_extensions);
  setupDebugMessenger();
  createSurface(init_info.window);
  createLogicalDevice(); // Also selects physical device
  ////createSwapChain();
  ////createImageViews();
  ////createRenderPass();
  ////createGraphicsPipeline();
  ////createFramebuffers();
  createCommandPool();
}

void VkWrapper::createSurface(GLFWwindow* window)
{
  // Create surface
  if (glfwCreateWindowSurface(instance_, window, nullptr, &surface_) !=
      VK_SUCCESS)
    throwVkErr("Failed to create window surface!");
}

void VkWrapper::destroy()
{

  // TODO: destroy everything
  // surface, devices, queues?
  vkDestroyDescriptorPool(device_, descriptor_pool_, allocator_);
#ifdef ELDR_VULKAN_DEBUG_REPORT
  auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(
    instance_, "vkDestroyDebugUtilsMessengerEXT");
  if (func != nullptr)
    func(instance_, debug_messenger_, allocator_);
#endif
  // Instance should be destroyed last
  vkDestroyInstance(instance_, allocator_);
}
} // Namespace vk_wrapper
} // Namespace eldr
