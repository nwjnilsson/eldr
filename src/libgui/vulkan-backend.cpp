#include <core/util.hpp>
#include <gui/gui.hpp>
#include <gui/vulkan-backend.hpp>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

#define VK_EXT_DEBUG_UTILS_NAME "VK_EXT_debug_utils"

namespace eldr {

void checkVkResult(VkResult result)
{
  if (result == 0)
    return;
  spdlog::error("[VULKAN] Error: VkResult = %d", result);
  if (result < 0)
    throw std::runtime_error("[VULKAN] Fatal error!");
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

VkPhysicalDevice vkSelectPhysicalDevice(VulkanData& vk_data)
{
  uint32_t gpu_count;
  VkResult err =
    vkEnumeratePhysicalDevices(vk_data.g_instance, &gpu_count, nullptr);
  checkVkResult(err);
  EASSERT(gpu_count > 0);
  std::vector<VkPhysicalDevice> gpus;
  gpus.resize(gpu_count);
  err = vkEnumeratePhysicalDevices(vk_data.g_instance, &gpu_count, gpus.data());
  checkVkResult(err);

  // Find discrete GPU if available
  for (VkPhysicalDevice& device : gpus) {
    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(device, &props);
    if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
      return device;
  }
  // Return integrated GPU if no discrete one is present
  return gpus[0];
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
#endif

int initVulkan(VulkanData&               vk_data,
               std::vector<const char*>& instance_extensions)
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
      throw std::runtime_error(
        "[VULKAN]: Validation layers requested, but not available!");
    }
    create_info.enabledLayerCount =
      static_cast<uint32_t>(validation_layers.size());
    create_info.ppEnabledLayerNames = validation_layers.data();
    instance_extensions.push_back(VK_EXT_DEBUG_UTILS_NAME);
#endif

    create_info.enabledExtensionCount =
      static_cast<uint32_t>(instance_extensions.size());
    create_info.ppEnabledExtensionNames = instance_extensions.data();

    err =
      vkCreateInstance(&create_info, vk_data.g_allocator, &vk_data.g_instance);

    checkVkResult(err);

    // Debug report callback
#ifdef ELDR_VULKAN_DEBUG_REPORT
    VkDebugUtilsMessengerCreateInfoEXT debug_report_ci{};

    debug_report_ci.sType =
      VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debug_report_ci.messageSeverity =
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debug_report_ci.messageType =
      VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debug_report_ci.pfnUserCallback = vkDebugReportCallback;
    debug_report_ci.pUserData       = nullptr; // Optional

    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(
      vk_data.g_instance, "vkCreateDebugUtilsMessengerEXT");
    EASSERT(func != nullptr);
    VkResult err = func(
      vk_data.g_instance, &debug_report_ci, vk_data.g_allocator,
      &vk_data.g_debug_messenger);
    checkVkResult(err);
#endif
  } // end private scope

  vk_data.g_physical_device = vkSelectPhysicalDevice(vk_data);

  // Select graphics queue family
  {
    uint32_t count;
    vkGetPhysicalDeviceQueueFamilyProperties(vk_data.g_physical_device, &count,
                                             nullptr);
    VkQueueFamilyProperties* queues = (VkQueueFamilyProperties*) malloc(
      sizeof(VkQueueFamilyProperties) * count);
    vkGetPhysicalDeviceQueueFamilyProperties(vk_data.g_physical_device, &count,
                                             queues);
    for (uint32_t i = 0; i < count; i++)
      if (queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
        vk_data.g_queue_family = i;
        break;
      }
    free(queues);
    EASSERT(vk_data.g_queue_family != (uint32_t) -1);
  }

  // Create Logical Device (with 1 queue)
  {
    std::vector<const char*> device_extensions;
    device_extensions.push_back("VK_KHR_swapchain");

    // Enumerate physical device extension
    uint32_t                           props_count;
    std::vector<VkExtensionProperties> properties;
    vkEnumerateDeviceExtensionProperties(vk_data.g_physical_device, nullptr,
                                         &props_count, nullptr);
    properties.resize(props_count);
    vkEnumerateDeviceExtensionProperties(vk_data.g_physical_device, nullptr,
                                         &props_count, properties.data());
#ifdef VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME
    if (IsExtensionAvailable(props, VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME))
      device_extensions.push_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
#endif

    const float             queue_priority[] = { 1.0f };
    VkDeviceQueueCreateInfo queue_info[1]    = {};
    queue_info[0].sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_info[0].queueFamilyIndex = vk_data.g_queue_family;
    queue_info[0].queueCount       = 1;
    queue_info[0].pQueuePriorities = queue_priority;
    VkDeviceCreateInfo create_info = {};
    create_info.sType              = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.queueCreateInfoCount =
      sizeof(queue_info) / sizeof(queue_info[0]);
    create_info.pQueueCreateInfos       = queue_info;
    create_info.enabledExtensionCount   = (uint32_t) device_extensions.size();
    create_info.ppEnabledExtensionNames = device_extensions.data();
    err = vkCreateDevice(vk_data.g_physical_device, &create_info,
                         vk_data.g_allocator, &vk_data.g_device);
    checkVkResult(err);
    vkGetDeviceQueue(vk_data.g_device, vk_data.g_queue_family, 0,
                     &vk_data.g_queue);
  }

  // Create Descriptor Pool
  // The example only requires a single combined image sampler descriptor for
  // the font image and only uses one descriptor set (for that) If you wish to
  // load e.g. additional textures you may need to alter pools sizes.
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
    err =
      vkCreateDescriptorPool(vk_data.g_device, &pool_info, vk_data.g_allocator,
                             &vk_data.g_descriptor_pool);
    checkVkResult(err);
  }

  return 0;
}

void setupVulkanWindow(VulkanData vk_data, VulkanWindow* window,
                       VkSurfaceKHR surface, int width, int height)
{
  window->surface = surface;

  VkBool32 res;
  vkGetPhysicalDeviceSurfaceSupportKHR(
    vk_data.g_physical_device, vk_data.g_queue_family, window->surface, &res);
  if (res != VK_TRUE) {
    spdlog::error("[VULKAN]: No WSI support on physical device 0");
    exit(-1);
  }

  // Select Surface Format
  const VkFormat requestSurfaceImageFormat[] = { VK_FORMAT_B8G8R8A8_UNORM,
                                                 VK_FORMAT_R8G8B8A8_UNORM,
                                                 VK_FORMAT_B8G8R8_UNORM,
                                                 VK_FORMAT_R8G8B8_UNORM };
  //  const VkColorSpaceKHR requestSurfaceColorSpace =
  //    VK_COLORSPACE_SRGB_NONLINEAR_KHR;
  //  window->surface_format = ImGui_ImplVulkanH_SelectSurfaceFormat(
  //    vk_data.g_physical_device, window->surface,
  //    requestSurfaceImageFormat, (size_t)
  //    IM_ARRAYSIZE(requestSurfaceImageFormat), requestSurfaceColorSpace);
  //
  //  // Select Present Mode
  // #ifdef ELDR_UNLIMITED_FRAME_RATE
  //  VkPresentModeKHR present_modes[] = { VK_PRESENT_MODE_MAILBOX_KHR,
  //                                       VK_PRESENT_MODE_IMMEDIATE_KHR,
  //                                       VK_PRESENT_MODE_FIFO_KHR };
  // #else
  //  VkPresentModeKHR present_modes[] = { VK_PRESENT_MODE_FIFO_KHR };
  // #endif
  //  window->present_mode = ImGui_ImplVulkanH_SelectPresentMode(
  //    vk_data.g_physical_device, window->surface, &present_modes[0],
  //    IM_ARRAYSIZE(present_modes));
  //  // printf("[vulkan] Selected PresentMode = %d\n", window->PresentMode);
  //
  //  // Create SwapChain, RenderPass, Framebuffer, etc.
  //  IM_ASSERT(vk_data.g_min_image_count >= 2);
  //  ImGui_ImplVulkanH_CreateOrResizeWindow(
  //    vk_data.g_instance, vk_data.g_physical_device,
  //    vk_data.g_device, window, vk_data.g_queue_family,
  //    vk_data.g_allocator, width, height,
  //    vk_data.g_min_image_count);
}
} // Namespace eldr
