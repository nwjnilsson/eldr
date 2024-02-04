#include <eldr/core/bitmap.hpp>
#include <eldr/core/logger.hpp>
#include <eldr/core/math.hpp>
#include <eldr/render/vulkan-wrapper.hpp>

#include <vulkan/vulkan_core.h>

#include <chrono> // TODO: remove once not used by updateUniformBuffer()
#include <filesystem>
#include <fstream>
#include <limits>
#include <optional>
#include <set>

#define VK_EXT_DEBUG_UTILS_NAME "VK_EXT_debug_utils"

namespace eldr {
namespace render {

// TYPES
struct SwapChainSupportDetails {
  VkSurfaceCapabilitiesKHR        capabilities;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR>   present_modes;
};

struct QueueFamilyIndices {
  std::optional<uint32_t> graphics_family;
  std::optional<uint32_t> present_family;
  bool                    isComplete()
  {
    return graphics_family.has_value() && present_family.has_value();
  }
};

struct UniformBufferObject {
  alignas(16) glm::mat4 model;
  alignas(16) glm::mat4 view;
  alignas(16) glm::mat4 proj;
};

struct VkData {
  VkData()
  {
    allocator       = nullptr;
    instance        = VK_NULL_HANDLE;
    physical_device = VK_NULL_HANDLE;
    device          = VK_NULL_HANDLE;
    present_queue   = VK_NULL_HANDLE;
    graphics_queue  = VK_NULL_HANDLE;
    debug_messenger = VK_NULL_HANDLE;
    pipeline_cache  = VK_NULL_HANDLE;
    descriptor_pool = VK_NULL_HANDLE;
    swapchain       = VK_NULL_HANDLE;
    surface         = VK_NULL_HANDLE;
    render_pass     = VK_NULL_HANDLE;
    // useDynamic_rendering= false;
    // clear_enable        = false;
    graphics_pipeline = VK_NULL_HANDLE;
    // frames           = nullptr;
    // frame_semaphores = nullptr;
    min_image_count   = 2;
    swapchain_rebuild = false;
    // surface_format      = VK_NULL_HANDLE;
    // present_mode        = VK_NULL_HANDLE;
    // clear_value         = VK_NULL_HANDLE;
    current_frame = 0;
  }

  VkAllocationCallbacks*       allocator;
  VkInstance                   instance;
  VkPhysicalDevice             physical_device;
  VkDevice                     device;
  VkQueue                      present_queue;
  VkQueue                      graphics_queue;
  VkDebugUtilsMessengerEXT     debug_messenger;
  VkPipelineCache              pipeline_cache;
  VkDescriptorPool             descriptor_pool;
  std::vector<VkDescriptorSet> descriptor_sets;

  GLFWwindow*              window;
  VkSwapchainKHR           swapchain;
  std::vector<VkImage>     swapchain_images;
  std::vector<VkImageView> swapchain_image_views;
  VkFormat                 swapchain_image_format;
  VkExtent2D               swapchain_extent;
  VkSurfaceKHR             surface;
  VkSurfaceFormatKHR       surface_format;
  VkPresentModeKHR         present_mode; //!
  // bool                     useDynamic_rendering;
  // bool                     clear_enable;
  // VkClearValue             clear_value;
  // uint32_t                 frame_index;
  uint32_t image_count;
  // uint32_t                 semaphore_index;
  std::vector<VkSemaphore>     image_available_sem;
  std::vector<VkSemaphore>     render_finished_sem;
  std::vector<VkFence>         in_flight_fences;
  uint32_t                     current_frame;
  VkPipeline                   graphics_pipeline;
  VkRenderPass                 render_pass;
  VkDescriptorSetLayout        descriptor_set_layout;
  VkPipelineLayout             pipeline_layout;
  std::vector<VkFramebuffer>   swapchain_framebuffers;
  VkCommandPool                command_pool;
  std::vector<VkCommandBuffer> command_buffers;
  // Frame*                     frames;
  // VulkanFrameSemaphores*     frame_semaphores;
  VkBuffer                    vertex_buffer;
  VkDeviceMemory              vertex_buffer_memory;
  VkBuffer                    index_buffer;
  VkDeviceMemory              index_buffer_memory;
  std::vector<VkBuffer>       uniform_buffers;
  std::vector<VkDeviceMemory> uniform_buffers_memory;
  std::vector<void*>          uniform_buffers_mapped;
  uint32_t                    min_image_count;
  bool                        swapchain_rebuild;

  // FUNCTIONS
  void updateUniformBuffer(uint32_t current_image);
  //
  void setupDebugMessenger();
  void createInstance(std::vector<const char*>& instance_extensions);
  void createSurface();
  void createLogicalDevice();
  void createSwapchain();
  void cleanupSwapchain();
  void recreateSwapchain();
  void createImageViews();
  void createRenderPass();
  void createDescriptorSetLayout();
  void createDescriptorPool();
  void createDescriptorSets();
  void createGraphicsPipeline();
  void createFramebuffers();
  void createCommandPool();
  void createTextureImage(); // TODO: remove
  void createVertexBuffer();
  void createIndexBuffer();
  void createUniformBuffers();
  void createBuffer(VkDeviceSize, VkBufferUsageFlags, VkMemoryPropertyFlags,
                    VkBuffer&, VkDeviceMemory&);
  void copyBuffer(VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size);
  void createCommandBuffers();
  void createSyncObjects();
  void recordCommandBuffer(uint32_t im_index);
  void drawFrame();
  void cleanup();
};

struct VkVertex {
  glm::vec2 pos;
  glm::vec3 color;

  static VkVertexInputBindingDescription getBindingDescription()
  {
    VkVertexInputBindingDescription binding_description{};
    binding_description.binding   = 0;
    binding_description.stride    = sizeof(VkVertex);
    binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return binding_description;
  }

  static std::array<VkVertexInputAttributeDescription, 2>
  getAttributeDescriptions()
  {
    std::array<VkVertexInputAttributeDescription, 2> attribute_descriptions{};
    attribute_descriptions[0].binding  = 0;
    attribute_descriptions[0].location = 0;
    attribute_descriptions[0].format   = VK_FORMAT_R32G32_SFLOAT;
    attribute_descriptions[0].offset   = offsetof(VkVertex, pos);
    attribute_descriptions[1].binding  = 0;
    attribute_descriptions[1].location = 1;
    attribute_descriptions[1].format   = VK_FORMAT_R32G32B32_SFLOAT;
    attribute_descriptions[1].offset   = offsetof(VkVertex, color);
    return attribute_descriptions;
  }
};

const std::vector<VkVertex> vertices = {
  { { -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f } },
  { { 0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f } },
  { { 0.5f, 0.5f }, { 0.0f, 0.0f, 1.0f } },
  { { -0.5f, 0.5f }, { 1.0f, 1.0f, 1.0f } }
};

const std::vector<uint16_t> indices = { 0, 1, 2, 2, 3, 0 };

// HELPERS AND DEVICE QUERYING FUNCTIONS
void throwVkErr(std::string msg)
{
  throw std::runtime_error("[VULKAN]: " + msg);
}

void checkVkResult(VkResult result)
{
  if (result == VK_SUCCESS)
    return;
  spdlog::error("[VULKAN] Error: VkResult = %d", result);
  if (result < 0)
    throwVkErr("Fatal error!");
}

// TODO: Maybe this should not be placed here
static std::vector<char> loadShader(const std::string& type)
{
  const char* env_p = std::getenv("ELDR_DIR");
  if (env_p == nullptr) {
    throw std::runtime_error("Environment not set up correctly");
  }

  std::string filename{};
  if (type == "vertex")
    filename = std::string(env_p) + "/resources/vert.spv";
  else if (type == "fragment")
    filename = std::string(env_p) + "/resources/frag.spv";

  std::ifstream file(filename, std::ios::ate | std::ios::binary);

  if (!file.is_open()) {
    throw std::runtime_error("[UTIL]: Failed to open file!");
  }

  size_t            file_size = (size_t) file.tellg();
  std::vector<char> buffer(file_size);
  file.seekg(0);
  file.read(buffer.data(), file_size);
  file.close();
  return buffer;
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
  SwapChainSupportDetails swapchain_support =
    swapChainSupportDetails(device, surface);

  return required_extensions.empty() && indices.isComplete() &&
         !swapchain_support.formats.empty() &&
         !swapchain_support.present_modes.empty();
}

VkSurfaceFormatKHR selectSwapSurfaceFormat(
  const std::vector<VkSurfaceFormatKHR>& available_formats)
{
  for (const auto& a_format : available_formats) {
    if (a_format.format == VK_FORMAT_B8G8R8_SRGB &&
        a_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      return a_format;
    }
  }
  return available_formats[0];
}

VkPresentModeKHR selectSwapPresentMode(
  const std::vector<VkPresentModeKHR>& available_present_modes)
{
  for (const auto& a_present_mode : available_present_modes) {
    if (a_present_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
      // prefer this if it exists (triple buffering)
      return VK_PRESENT_MODE_MAILBOX_KHR;
    }
  }
  return VK_PRESENT_MODE_FIFO_KHR; // guaranteed to exist (vertical sync)
}

VkExtent2D selectSwapExtent(GLFWwindow*                     window,
                            const VkSurfaceCapabilitiesKHR& capabilities)
{
  // Match the resolution of the current window, unless the value is the max
  // of uint32_t
  if (capabilities.currentExtent.width !=
      std::numeric_limits<uint32_t>::max()) {
    return capabilities.currentExtent;
  }
  else {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    VkExtent2D extent = { static_cast<uint32_t>(width),
                          static_cast<uint32_t>(height) };
    extent.width = std::clamp(extent.width, capabilities.minImageExtent.width,
                              capabilities.maxImageExtent.width);
    extent.height =
      std::clamp(extent.height, capabilities.minImageExtent.height,
                 capabilities.maxImageExtent.height);
    return extent;
  }
}

uint32_t findMemoryType(VkPhysicalDevice& device, uint32_t type_filter,
                        VkMemoryPropertyFlags properties)
{
  VkPhysicalDeviceMemoryProperties mem_props;
  vkGetPhysicalDeviceMemoryProperties(device, &mem_props);
  for (uint32_t i = 0; i < mem_props.memoryTypeCount; ++i) {
    if (type_filter & (1 << i) &&
        (mem_props.memoryTypes[i].propertyFlags & properties) == properties) {
      return i;
    }
  }
  throwVkErr("Failed to find suitable memory type!");
  return -1;
}

void selectPhysicalDevice(VkInstance instance, VkSurfaceKHR surface,
                          VkPhysicalDevice&         physical_device,
                          std::vector<const char*>& device_extensions)
{
  uint32_t gpu_count;
  VkResult err = vkEnumeratePhysicalDevices(instance, &gpu_count, nullptr);
  checkVkResult(err);

  if (gpu_count == 0)
    throwVkErr("No compatible device found");

  std::vector<VkPhysicalDevice> gpus;
  gpus.resize(gpu_count);
  err = vkEnumeratePhysicalDevices(instance, &gpu_count, gpus.data());
  checkVkResult(err);

  // Find discrete GPU if available
  // It is possible to add additional criteria for what is to be considered
  // a suitable GPU. The Vulkan programming tutorial suggests implementing a
  // scoring system where each usable property of a device would add to its
  // overall score, and when the properties of all devices have been
  // evaluated, the highest scoring device is considered the most suitable
  // one. For now I don't really think it matters. Just use a discrete GPU if
  // it's available.
  for (VkPhysicalDevice& device : gpus) {
    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(device, &props);
    if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
        isDeviceSuitable(device, surface, device_extensions)) {
      physical_device = device;
      return;
    }
  }
  // Return integrated GPU if no discrete one is present
  physical_device = gpus[0];
}

// ------------------------------- VkData --------------------------------------

void VkData::createInstance(std::vector<const char*>& instance_extensions)
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

    err = vkCreateInstance(&create_info, allocator, &instance);
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

void VkData::setupDebugMessenger()
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
    instance, "vkCreateDebugUtilsMessengerEXT");
  assert(func != nullptr);
  VkResult err = func(instance, &debug_report_ci, allocator, &debug_messenger);
  checkVkResult(err);
}
#endif

void VkData::createLogicalDevice()
{
  std::vector<const char*> device_extensions;
  device_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
#ifdef VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME
  device_extensions.push_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
#endif

  selectPhysicalDevice(instance, surface, physical_device, device_extensions);

  QueueFamilyIndices indices = findQueueFamilies(physical_device, surface);
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
    vkCreateDevice(physical_device, &create_info, allocator, &device);
  checkVkResult(err);

  vkGetDeviceQueue(device, indices.present_family.value(), 0, &present_queue);
  vkGetDeviceQueue(device, indices.graphics_family.value(), 0, &graphics_queue);
}

void VkData::createSwapchain()
{
  SwapChainSupportDetails swapchain_support =
    swapChainSupportDetails(physical_device, surface);
  surface_format    = selectSwapSurfaceFormat(swapchain_support.formats);
  present_mode      = selectSwapPresentMode(swapchain_support.present_modes);
  VkExtent2D extent = selectSwapExtent(window, swapchain_support.capabilities);

  // TODO: keep this image count?
  min_image_count = swapchain_support.capabilities.minImageCount + 1;
  // If there is an upper limit, make sure we don't exceed it
  if (swapchain_support.capabilities.maxImageCount > 0) {
    min_image_count =
      std::min(min_image_count, swapchain_support.capabilities.maxImageCount);
  }
  VkSwapchainCreateInfoKHR swap_ci{};
  swap_ci.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  swap_ci.surface          = surface;
  swap_ci.minImageCount    = min_image_count;
  swap_ci.imageFormat      = surface_format.format;
  swap_ci.imageColorSpace  = surface_format.colorSpace;
  swap_ci.imageExtent      = extent;
  swap_ci.imageArrayLayers = 1;
  swap_ci.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  swap_ci.preTransform     = swapchain_support.capabilities.currentTransform;
  swap_ci.compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  swap_ci.presentMode      = present_mode;
  swap_ci.clipped      = VK_TRUE; // don't care about color of obscured pixels
  swap_ci.oldSwapchain = VK_NULL_HANDLE;

  QueueFamilyIndices indices = findQueueFamilies(physical_device, surface);
  uint32_t           queueFamilyIndices[] = { indices.graphics_family.value(),
                                              indices.present_family.value() };

  if (indices.graphics_family != indices.present_family) {
    swap_ci.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
    swap_ci.queueFamilyIndexCount = 2;
    swap_ci.pQueueFamilyIndices   = queueFamilyIndices;
  }
  else {
    swap_ci.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
    swap_ci.queueFamilyIndexCount = 0;       // Optional
    swap_ci.pQueueFamilyIndices   = nullptr; // Optional
  }

  if (vkCreateSwapchainKHR(device, &swap_ci, allocator, &swapchain) !=
      VK_SUCCESS) {
    throwVkErr("Failed to create swapchain");
  }
  vkGetSwapchainImagesKHR(device, swapchain, &image_count, nullptr);
  swapchain_images.resize(image_count);
  vkGetSwapchainImagesKHR(device, swapchain, &image_count,
                          swapchain_images.data());
  swapchain_extent       = extent;
  swapchain_image_format = surface_format.format;
}

void VkData::cleanupSwapchain()
{
  for (size_t i = 0; i < swapchain_framebuffers.size(); ++i)
    vkDestroyFramebuffer(device, swapchain_framebuffers[i], allocator);
  for (size_t i = 0; i < swapchain_image_views.size(); ++i)
    vkDestroyImageView(device, swapchain_image_views[i], allocator);

  vkDestroySwapchainKHR(device, swapchain, allocator);
}

void VkData::recreateSwapchain()
{
  vkDeviceWaitIdle(device);
  cleanupSwapchain();
  createSwapchain();
  createImageViews();
  createFramebuffers();
}

void VkData::createImageViews()
{
  swapchain_image_views.resize(swapchain_images.size());
  for (size_t i = 0; i < swapchain_images.size(); i++) {
    VkImageViewCreateInfo image_view_ci{};
    image_view_ci.sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    image_view_ci.image    = swapchain_images[i];
    image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_ci.format   = swapchain_image_format;
    // Standard color properties
    image_view_ci.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_ci.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_ci.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_ci.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

    image_view_ci.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    image_view_ci.subresourceRange.baseMipLevel   = 0;
    image_view_ci.subresourceRange.levelCount     = 1;
    image_view_ci.subresourceRange.baseArrayLayer = 0;
    image_view_ci.subresourceRange.layerCount     = 1;

    if (vkCreateImageView(device, &image_view_ci, allocator,
                          &swapchain_image_views[i]) != VK_SUCCESS) {
      throwVkErr("Failed to create image view!");
    }
  }
}

void VkData::createRenderPass()
{
  VkAttachmentDescription color_attachment{};
  color_attachment.format         = swapchain_image_format;
  color_attachment.samples        = VK_SAMPLE_COUNT_1_BIT;
  color_attachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
  color_attachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
  color_attachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  color_attachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
  color_attachment.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentReference color_attachment_ref{};
  color_attachment_ref.attachment = 0;
  color_attachment_ref.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass{};
  subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments    = &color_attachment_ref;

  VkSubpassDependency dependency{};
  dependency.srcSubpass    = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass    = 0;
  dependency.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.srcAccessMask = 0;
  dependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

  VkRenderPassCreateInfo render_pass_ci{};
  render_pass_ci.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  render_pass_ci.attachmentCount = 1;
  render_pass_ci.pAttachments    = &color_attachment;
  render_pass_ci.subpassCount    = 1;
  render_pass_ci.pSubpasses      = &subpass;
  render_pass_ci.dependencyCount = 1;
  render_pass_ci.pDependencies   = &dependency;

  if (vkCreateRenderPass(device, &render_pass_ci, allocator, &render_pass) !=
      VK_SUCCESS) {
    throwVkErr("Failed to create render pass!");
  }
}

VkShaderModule createShaderModule(VkDevice&                device,
                                  VkAllocationCallbacks*   allocator,
                                  const std::vector<char>& bytecode)
{
  VkShaderModuleCreateInfo shader_ci{};
  shader_ci.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  shader_ci.codeSize = bytecode.size();
  shader_ci.pCode    = reinterpret_cast<const uint32_t*>(bytecode.data());

  VkShaderModule shader_module;
  if (vkCreateShaderModule(device, &shader_ci, allocator, &shader_module) !=
      VK_SUCCESS) {
    throwVkErr("Failed to create shader module!");
  }
  return shader_module;
}

void VkData::createDescriptorSetLayout()
{
  VkDescriptorSetLayoutBinding ubo_layout_binding{};
  ubo_layout_binding.binding            = 0;
  ubo_layout_binding.descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  ubo_layout_binding.descriptorCount    = 1;
  ubo_layout_binding.stageFlags         = VK_SHADER_STAGE_VERTEX_BIT;
  ubo_layout_binding.pImmutableSamplers = nullptr; // optional

  VkDescriptorSetLayoutCreateInfo layout_ci{};
  layout_ci.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layout_ci.bindingCount = 1;
  layout_ci.pBindings    = &ubo_layout_binding;

  if (vkCreateDescriptorSetLayout(device, &layout_ci, allocator,
                                  &descriptor_set_layout) != VK_SUCCESS)
    throwVkErr("Failed to create descriptor set layout!");
}

void VkData::createDescriptorPool()
{

  VkDescriptorPoolSize pool_size{};
  pool_size.type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  pool_size.descriptorCount = static_cast<uint32_t>(max_frames_in_flight);

  VkDescriptorPoolCreateInfo pool_ci{};
  pool_ci.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  pool_ci.poolSizeCount = 1;
  pool_ci.pPoolSizes    = &pool_size;
  pool_ci.maxSets       = static_cast<uint32_t>(max_frames_in_flight);

  if (vkCreateDescriptorPool(device, &pool_ci, allocator, &descriptor_pool) !=
      VK_SUCCESS)
    throwVkErr("Failed to create descriptor pool!");
}

void VkData::createDescriptorSets()
{
  std::vector<VkDescriptorSetLayout> layouts(max_frames_in_flight,
                                             descriptor_set_layout);

  VkDescriptorSetAllocateInfo alloc_info{};
  alloc_info.sType          = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  alloc_info.descriptorPool = descriptor_pool;
  alloc_info.descriptorSetCount = static_cast<uint32_t>(max_frames_in_flight);
  alloc_info.pSetLayouts        = layouts.data();

  descriptor_sets.resize(max_frames_in_flight);
  if (vkAllocateDescriptorSets(device, &alloc_info, descriptor_sets.data()) !=
      VK_SUCCESS)
    throwVkErr("Failed to allocate descriptor sets!");

  for (size_t i = 0; i < max_frames_in_flight; ++i) {
    VkDescriptorBufferInfo buffer_info{};
    buffer_info.buffer = uniform_buffers[i];
    buffer_info.offset = 0;
    buffer_info.range  = sizeof(UniformBufferObject);

    VkWriteDescriptorSet descriptor_write{};
    descriptor_write.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_write.dstSet          = descriptor_sets[i];
    descriptor_write.dstBinding      = 0;
    descriptor_write.dstArrayElement = 0;
    descriptor_write.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_write.descriptorCount = 1;
    // Either pBufferInfo or pImageInfo or pTexelBufferView is set
    descriptor_write.pBufferInfo      = &buffer_info;
    descriptor_write.pImageInfo       = nullptr;
    descriptor_write.pTexelBufferView = nullptr;

    vkUpdateDescriptorSets(device, 1, &descriptor_write, 0, nullptr);
  }
}

void VkData::createGraphicsPipeline()
{
  std::vector<char> vert_shader = loadShader("vertex");
  std::vector<char> frag_shader = loadShader("fragment");
  spdlog::info("Vertex shader size = {}", vert_shader.size());
  spdlog::info("Fragment shader size = {}", frag_shader.size());

  VkShaderModule vert_shader_module =
    createShaderModule(device, allocator, vert_shader);
  VkShaderModule frag_shader_module =
    createShaderModule(device, allocator, frag_shader);

  VkPipelineShaderStageCreateInfo vert_shader_stage_ci{};
  vert_shader_stage_ci.sType =
    VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vert_shader_stage_ci.stage  = VK_SHADER_STAGE_VERTEX_BIT;
  vert_shader_stage_ci.module = vert_shader_module;
  vert_shader_stage_ci.pName  = "main";
  // The variable below is useful for constants in the shader. Can be more
  // efficient than configuring constants at render time.
  vert_shader_stage_ci.pSpecializationInfo = nullptr;

  VkPipelineShaderStageCreateInfo frag_shader_stage_ci{};
  frag_shader_stage_ci.sType =
    VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  frag_shader_stage_ci.stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
  frag_shader_stage_ci.module = frag_shader_module;
  frag_shader_stage_ci.pName  = "main";
  // The variable below is useful for constants in the shader. Can be more
  // efficient than configuring constants at render time.
  frag_shader_stage_ci.pSpecializationInfo = nullptr;

  VkPipelineShaderStageCreateInfo shader_stages[] = { vert_shader_stage_ci,
                                                      frag_shader_stage_ci };

  VkPipelineVertexInputStateCreateInfo vertex_input_info{};
  auto binding_description    = VkVertex::getBindingDescription();
  auto attribute_descriptions = VkVertex::getAttributeDescriptions();
  vertex_input_info.sType =
    VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertex_input_info.vertexBindingDescriptionCount = 1;
  vertex_input_info.pVertexBindingDescriptions    = &binding_description;
  vertex_input_info.vertexAttributeDescriptionCount =
    static_cast<uint32_t>(attribute_descriptions.size());
  vertex_input_info.pVertexAttributeDescriptions =
    attribute_descriptions.data();

  VkPipelineInputAssemblyStateCreateInfo input_assembly{};
  input_assembly.sType =
    VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  // TODO: Use different topology?
  input_assembly.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  input_assembly.primitiveRestartEnable = VK_FALSE;

  VkViewport viewport{};
  viewport.x        = 0.0f;
  viewport.y        = 0.0f;
  viewport.width    = (float) swapchain_extent.width;
  viewport.height   = (float) swapchain_extent.height;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  VkRect2D scissor{};
  scissor.offset = { 0, 0 };
  scissor.extent = swapchain_extent;

  std::vector<VkDynamicState> dynamic_states = { VK_DYNAMIC_STATE_VIEWPORT,
                                                 VK_DYNAMIC_STATE_SCISSOR };

  VkPipelineDynamicStateCreateInfo dynamic_state{};
  dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamic_state.dynamicStateCount =
    static_cast<uint32_t>(dynamic_states.size());
  dynamic_state.pDynamicStates = dynamic_states.data();

  VkPipelineViewportStateCreateInfo viewport_state{};
  viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewport_state.viewportCount = 1;
  viewport_state.pViewports    = &viewport;
  viewport_state.scissorCount  = 1;
  viewport_state.pScissors     = &scissor;

  VkPipelineRasterizationStateCreateInfo rasterizer{};
  rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterizer.depthClampEnable        = VK_FALSE;
  rasterizer.rasterizerDiscardEnable = VK_FALSE;
  rasterizer.polygonMode             = VK_POLYGON_MODE_FILL;
  rasterizer.lineWidth               = 1.0f;
  rasterizer.cullMode                = VK_CULL_MODE_BACK_BIT;
  rasterizer.frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE;
  rasterizer.depthBiasEnable         = VK_FALSE;

  VkPipelineMultisampleStateCreateInfo multisampling{};
  multisampling.sType =
    VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisampling.sampleShadingEnable   = VK_FALSE;
  multisampling.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT;
  multisampling.minSampleShading      = 1.0f;     // Optional
  multisampling.pSampleMask           = nullptr;  // Optional
  multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
  multisampling.alphaToOneEnable      = VK_FALSE; // Optional

  // TODO: may want to change
  VkPipelineColorBlendAttachmentState colorBlendAttachment{};
  colorBlendAttachment.colorWriteMask =
    VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
    VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  colorBlendAttachment.blendEnable         = VK_TRUE;
  colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
  colorBlendAttachment.dstColorBlendFactor =
    VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  colorBlendAttachment.colorBlendOp        = VK_BLEND_OP_ADD;
  colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
  colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  colorBlendAttachment.alphaBlendOp        = VK_BLEND_OP_ADD;

  // TODO: specify
  VkPipelineColorBlendStateCreateInfo color_blending{};
  color_blending.sType =
    VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  color_blending.logicOpEnable     = VK_FALSE;
  color_blending.logicOp           = VK_LOGIC_OP_COPY; // Optional
  color_blending.attachmentCount   = 1;
  color_blending.pAttachments      = &colorBlendAttachment;
  color_blending.blendConstants[0] = 0.0f; // Optional
  color_blending.blendConstants[1] = 0.0f; // Optional
  color_blending.blendConstants[2] = 0.0f; // Optional
  color_blending.blendConstants[3] = 0.0f; // Optional

  VkPipelineLayoutCreateInfo pipeline_layout_ci{};
  pipeline_layout_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipeline_layout_ci.setLayoutCount         = 1;
  pipeline_layout_ci.pSetLayouts            = &descriptor_set_layout;
  pipeline_layout_ci.pushConstantRangeCount = 0;       // Optional
  pipeline_layout_ci.pPushConstantRanges    = nullptr; // Optional

  if (vkCreatePipelineLayout(device, &pipeline_layout_ci, nullptr,
                             &pipeline_layout) != VK_SUCCESS) {
    throwVkErr("Failed to create pipeline layout");
  }

  VkGraphicsPipelineCreateInfo pipeline_ci{};
  pipeline_ci.sType      = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipeline_ci.stageCount = 2;
  pipeline_ci.pStages    = shader_stages;
  pipeline_ci.pVertexInputState   = &vertex_input_info;
  pipeline_ci.pInputAssemblyState = &input_assembly;
  pipeline_ci.pViewportState      = &viewport_state;
  pipeline_ci.pRasterizationState = &rasterizer;
  pipeline_ci.pMultisampleState   = &multisampling;
  pipeline_ci.pDepthStencilState  = nullptr;
  pipeline_ci.pColorBlendState    = &color_blending;
  pipeline_ci.pDynamicState       = &dynamic_state;
  pipeline_ci.layout              = pipeline_layout;
  pipeline_ci.renderPass          = render_pass;
  pipeline_ci.subpass             = 0;
  pipeline_ci.basePipelineHandle  = VK_NULL_HANDLE; // Optional
  pipeline_ci.basePipelineIndex   = -1;             // Optional

  if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipeline_ci,
                                nullptr, &graphics_pipeline) != VK_SUCCESS) {
    throwVkErr("Failed to create graphics pipeline!");
  }

  vkDestroyShaderModule(device, vert_shader_module, allocator);
  vkDestroyShaderModule(device, frag_shader_module, allocator);
}

void VkData::createFramebuffers()
{
  swapchain_framebuffers.resize(swapchain_image_views.size());
  for (size_t i = 0; i < swapchain_image_views.size(); ++i) {
    VkImageView             attachments[] = { swapchain_image_views[i] };
    VkFramebufferCreateInfo framebuffer_ci{};
    framebuffer_ci.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebuffer_ci.renderPass      = render_pass;
    framebuffer_ci.attachmentCount = 1;
    framebuffer_ci.pAttachments    = attachments;
    framebuffer_ci.width           = swapchain_extent.width;
    framebuffer_ci.height          = swapchain_extent.height;
    framebuffer_ci.layers          = 1;

    if (vkCreateFramebuffer(device, &framebuffer_ci, allocator,
                            &swapchain_framebuffers[i]) != VK_SUCCESS)
      throwVkErr("Failed to create framebuffer!");
  }
}

void VkData::createCommandPool()
{
  QueueFamilyIndices queue_family_indices =
    findQueueFamilies(physical_device, surface);

  VkCommandPoolCreateInfo pool_ci = {};
  pool_ci.sType                   = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  pool_ci.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  pool_ci.queueFamilyIndex = queue_family_indices.graphics_family.value();
  if (vkCreateCommandPool(device, &pool_ci, allocator, &command_pool) !=
      VK_SUCCESS)
    throwVkErr("Failed to create command pool!");
}

void VkData::createTextureImage()
{

  const char* env_p = std::getenv("ELDR_DIR");
  if (env_p == nullptr) {
    throw std::runtime_error("Environment not set up correctly");
  }
  std::string filename = std::string(env_p) + "/resources/texture.jpg";

  // use bitmap class somehow
}

void VkData::createCommandBuffers()
{
  VkCommandBufferAllocateInfo alloc_info{};
  alloc_info.sType       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  alloc_info.commandPool = command_pool;
  alloc_info.level       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  alloc_info.commandBufferCount = max_frames_in_flight;
  command_buffers.resize(max_frames_in_flight);

  if (vkAllocateCommandBuffers(device, &alloc_info, command_buffers.data()) !=
      VK_SUCCESS)
    throwVkErr("Failed to create command buffer!");
}

void VkData::createSurface()
{
  // Create surface
  if (glfwCreateWindowSurface(instance, window, allocator, &surface) !=
      VK_SUCCESS)
    throwVkErr("Failed to create window surface!");
}

void VkData::createSyncObjects()
{
  image_available_sem.resize(max_frames_in_flight);
  render_finished_sem.resize(max_frames_in_flight);
  in_flight_fences.resize(max_frames_in_flight);

  VkSemaphoreCreateInfo semaphore_ci{};
  semaphore_ci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  VkFenceCreateInfo fence_ci{};
  fence_ci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fence_ci.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  for (size_t i = 0; i < max_frames_in_flight; ++i) {
    if (vkCreateSemaphore(device, &semaphore_ci, allocator,
                          &image_available_sem[i]) != VK_SUCCESS ||
        vkCreateSemaphore(device, &semaphore_ci, allocator,
                          &render_finished_sem[i]) != VK_SUCCESS ||
        vkCreateFence(device, &fence_ci, allocator, &in_flight_fences[i]) !=
          VK_SUCCESS)
      throwVkErr("Failed to create sync objects!");
  }
}

void VkData::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                          VkMemoryPropertyFlags properties, VkBuffer& buffer,
                          VkDeviceMemory& buffer_memory)
{
  VkBufferCreateInfo buffer_ci{};
  buffer_ci.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  buffer_ci.size        = size;
  buffer_ci.usage       = usage;
  buffer_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  if (vkCreateBuffer(device, &buffer_ci, allocator, &buffer) != VK_SUCCESS)
    throwVkErr("Failed to create buffer!");

  VkMemoryRequirements mem_requirements;
  vkGetBufferMemoryRequirements(device, buffer, &mem_requirements);

  VkMemoryAllocateInfo alloc_info{};
  alloc_info.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  alloc_info.allocationSize  = mem_requirements.size;
  alloc_info.memoryTypeIndex = findMemoryType(
    physical_device, mem_requirements.memoryTypeBits, properties);

  if (vkAllocateMemory(device, &alloc_info, allocator, &buffer_memory) !=
      VK_SUCCESS)
    throwVkErr("Failed to allocate buffer memory!");

  vkBindBufferMemory(device, buffer, buffer_memory, 0);
}

void VkData::createVertexBuffer()
{
  VkDeviceSize buffer_size = sizeof(vertices[0]) * vertices.size();

  VkBuffer       staging_buffer;
  VkDeviceMemory staging_buffer_memory;
  createBuffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
               staging_buffer, staging_buffer_memory);

  void* data;
  vkMapMemory(device, staging_buffer_memory, 0, buffer_size, 0, &data);
  memcpy(data, vertices.data(), (size_t) buffer_size);
  vkUnmapMemory(device, staging_buffer_memory);

  createBuffer(
    buffer_size,
    VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertex_buffer, vertex_buffer_memory);

  copyBuffer(staging_buffer, vertex_buffer, buffer_size);

  vkDestroyBuffer(device, staging_buffer, allocator);
  vkFreeMemory(device, staging_buffer_memory, allocator);
}

// TODO: Merge index and vertex buffer into a single buffer and use offsets
// Should be more cache friendly
void VkData::createIndexBuffer()
{

  VkDeviceSize   buffer_size = sizeof(indices[0]) * indices.size();
  VkBuffer       staging_buffer;
  VkDeviceMemory staging_buffer_memory;
  createBuffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
               staging_buffer, staging_buffer_memory);
  void* data;
  vkMapMemory(device, staging_buffer_memory, 0, buffer_size, 0, &data);
  memcpy(data, indices.data(), (size_t) buffer_size);
  vkUnmapMemory(device, staging_buffer_memory);

  createBuffer(
    buffer_size,
    VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, index_buffer, index_buffer_memory);
  copyBuffer(staging_buffer, index_buffer, buffer_size);
  vkDestroyBuffer(device, staging_buffer, allocator);
  vkFreeMemory(device, staging_buffer_memory, allocator);
}

void VkData::createUniformBuffers()
{

  VkDeviceSize buffer_size = sizeof(UniformBufferObject);
  uniform_buffers.resize(max_frames_in_flight);
  uniform_buffers_memory.resize(max_frames_in_flight);
  uniform_buffers_mapped.resize(max_frames_in_flight);

  for (size_t i = 0; i < max_frames_in_flight; ++i) {
    createBuffer(buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                   VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 uniform_buffers[i], uniform_buffers_memory[i]);

    vkMapMemory(device, uniform_buffers_memory[i], 0, buffer_size, 0,
                &uniform_buffers_mapped[i]);
  }
}

// TODO: may want to optimize this function if used often (use fences instead of
// vkQueueWaitIdle)
void VkData::copyBuffer(VkBuffer src_buffer, VkBuffer dst_buffer,
                        VkDeviceSize size)
{
  VkCommandBufferAllocateInfo alloc_info{};
  alloc_info.sType       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  alloc_info.level       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  alloc_info.commandPool = command_pool;
  alloc_info.commandBufferCount = 1;

  VkCommandBuffer command_buffer;
  vkAllocateCommandBuffers(device, &alloc_info, &command_buffer);

  VkCommandBufferBeginInfo begin_info{};
  begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  vkBeginCommandBuffer(command_buffer, &begin_info);

  VkBufferCopy copy_region{}; // There are optional offsets in this struct
  copy_region.size = size;
  vkCmdCopyBuffer(command_buffer, src_buffer, dst_buffer, 1, &copy_region);

  vkEndCommandBuffer(command_buffer);

  VkSubmitInfo submit_info{};
  submit_info.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers    = &command_buffer;
  vkQueueSubmit(graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
  vkQueueWaitIdle(graphics_queue);

  vkFreeCommandBuffers(device, command_pool, 1, &command_buffer);
}

void VkData::recordCommandBuffer(uint32_t image_index)
{
  VkCommandBufferBeginInfo command_buffer_info{};
  command_buffer_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  command_buffer_info.flags = 0;
  command_buffer_info.pInheritanceInfo = nullptr;

  if (vkBeginCommandBuffer(command_buffers[current_frame],
                           &command_buffer_info) != VK_SUCCESS)
    throwVkErr("Failed to create command buffer!");

  VkRenderPassBeginInfo render_pass_info{};
  render_pass_info.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  render_pass_info.renderPass        = render_pass;
  render_pass_info.framebuffer       = swapchain_framebuffers[image_index];
  render_pass_info.renderArea.offset = { 0, 0 };
  render_pass_info.renderArea.extent = swapchain_extent;

  VkClearValue clear_color         = { { { 0.0f, 0.0f, 0.0f, 1.0f } } };
  render_pass_info.clearValueCount = 1;
  render_pass_info.pClearValues    = &clear_color;

  vkCmdBeginRenderPass(command_buffers[current_frame], &render_pass_info,
                       VK_SUBPASS_CONTENTS_INLINE);

  vkCmdBindPipeline(command_buffers[current_frame],
                    VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline);
  VkViewport viewport{};
  viewport.x        = 0.0f;
  viewport.y        = 0.0f;
  viewport.width    = static_cast<float>(swapchain_extent.width);
  viewport.height   = static_cast<float>(swapchain_extent.height);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  vkCmdSetViewport(command_buffers[current_frame], 0, 1, &viewport);

  VkRect2D scissor{};
  scissor.offset = { 0, 0 };
  scissor.extent = swapchain_extent;
  vkCmdSetScissor(command_buffers[current_frame], 0, 1, &scissor);

  vkCmdBindPipeline(command_buffers[current_frame],
                    VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline);

  VkBuffer     vertex_buffers[] = { vertex_buffer };
  VkDeviceSize offsets[]        = { 0 };

  vkCmdBindVertexBuffers(command_buffers[current_frame], 0, 1, vertex_buffers,
                         offsets);
  vkCmdBindIndexBuffer(command_buffers[current_frame], index_buffer, 0,
                       VK_INDEX_TYPE_UINT16);

  vkCmdBindDescriptorSets(command_buffers[current_frame],
                          VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0,
                          1, &descriptor_sets[current_frame], 0, nullptr);

  vkCmdDrawIndexed(command_buffers[current_frame],
                   static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

  vkCmdEndRenderPass(command_buffers[current_frame]);
  if (vkEndCommandBuffer(command_buffers[current_frame]) != VK_SUCCESS)
    throwVkErr("Failed to record command buffer!");
}

// TODO: remove
void VkData::updateUniformBuffer(uint32_t current_image)
{

  static auto start_time = std::chrono::high_resolution_clock::now();

  auto current_time = std::chrono::high_resolution_clock::now();

  float time = std::chrono::duration<float, std::chrono::seconds::period>(
                 current_time - start_time)
                 .count();

  UniformBufferObject ubo{};
  ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f),
                          glm::vec3(0.0f, 0.0f, 1.0f));

  ubo.view =
    glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                glm::vec3(0.0f, 0.0f, 1.0f));

  ubo.proj = glm::perspective(
    glm::radians(45.0f),
    swapchain_extent.width / (float) swapchain_extent.height, 0.1f, 10.0f);

  ubo.proj[1][1] *= -1;

  memcpy(uniform_buffers_mapped[current_image], &ubo, sizeof(ubo));
}
void VkData::drawFrame()
{

  vkWaitForFences(device, 1, &in_flight_fences[current_frame], VK_TRUE,
                  UINT64_MAX);
  vkResetFences(device, 1, &in_flight_fences[current_frame]);

  uint32_t image_index;
  vkAcquireNextImageKHR(device, swapchain, UINT64_MAX,
                        image_available_sem[current_frame], VK_NULL_HANDLE,
                        &image_index);
  vkResetCommandBuffer(command_buffers[current_frame], 0);

  recordCommandBuffer(image_index);

  updateUniformBuffer(current_frame);

  VkSubmitInfo submit_info{};
  submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

  VkSemaphore wait_semaphores[]      = { image_available_sem[current_frame] };
  VkPipelineStageFlags wait_stages[] = {
    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
  };
  submit_info.waitSemaphoreCount   = 1;
  submit_info.pWaitSemaphores      = wait_semaphores;
  submit_info.pWaitDstStageMask    = wait_stages;
  submit_info.commandBufferCount   = 1;
  submit_info.pCommandBuffers      = &command_buffers[current_frame];
  VkSemaphore signal_semaphores[]  = { render_finished_sem[current_frame] };
  submit_info.signalSemaphoreCount = 1;
  submit_info.pSignalSemaphores    = signal_semaphores;

  if (vkQueueSubmit(graphics_queue, 1, &submit_info,
                    in_flight_fences[current_frame]) != VK_SUCCESS)
    throwVkErr("Failed to submit draw command buffer!");

  VkPresentInfoKHR present_info{};
  present_info.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  present_info.waitSemaphoreCount = 1;
  present_info.pWaitSemaphores    = signal_semaphores;

  VkSwapchainKHR swapchains[] = { swapchain };
  present_info.swapchainCount = 1;
  present_info.pSwapchains    = swapchains;
  present_info.pImageIndices  = &image_index;

  vkQueuePresentKHR(present_queue, &present_info);

  current_frame = (current_frame + 1) % max_frames_in_flight;
}

void VkData::cleanup()
{
  vkDeviceWaitIdle(device);

  // Sync objects
  for (size_t i = 0; i < max_frames_in_flight; ++i) {
    vkDestroySemaphore(device, image_available_sem[i], allocator);
    vkDestroySemaphore(device, render_finished_sem[i], allocator);
    vkDestroyFence(device, in_flight_fences[i], allocator);
  }

  vkDestroyCommandPool(device, command_pool, allocator);

  cleanupSwapchain();

  for (size_t i = 0; i < max_frames_in_flight; ++i) {
    vkDestroyBuffer(device, uniform_buffers[i], allocator);
    vkFreeMemory(device, uniform_buffers_memory[i], allocator);
  }

  vkDestroyBuffer(device, index_buffer, allocator);
  vkFreeMemory(device, index_buffer_memory, allocator);
  vkDestroyBuffer(device, vertex_buffer, allocator);
  vkFreeMemory(device, vertex_buffer_memory, allocator);

  vkDestroyPipeline(device, graphics_pipeline, allocator);
  vkDestroyPipelineLayout(device, pipeline_layout, allocator);
  vkDestroyRenderPass(device, render_pass, allocator);
  vkDestroyDescriptorPool(device, descriptor_pool, allocator);
  vkDestroyDescriptorSetLayout(device, descriptor_set_layout, allocator);
  vkDestroySurfaceKHR(instance, surface, allocator);
  vkDestroyDevice(device, allocator);

#ifdef ELDR_VULKAN_DEBUG_REPORT
  auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(
    instance, "vkDestroyDebugUtilsMessengerEXT");
  if (func != nullptr)
    func(instance, debug_messenger, allocator);
#endif
  // Instance should be destroyed last
  vkDestroyInstance(instance, allocator);
}

// ----------------------------- VkWrapper -------------------------------------

VkWrapper::VkWrapper() { vk_data_ = std::make_unique<VkData>(); }
VkWrapper::~VkWrapper() { vk_data_->cleanup(); }

void VkWrapper::init(VkWrapperInitInfo& init_info)
{
  vk_data_->window = init_info.window;
  vk_data_->createInstance(init_info.instance_extensions);
#ifdef ELDR_VULKAN_DEBUG_REPORT
  vk_data_->setupDebugMessenger();
#endif
  vk_data_->createSurface();
  vk_data_->createLogicalDevice(); // Also selects physical device
  vk_data_->createSwapchain();
  vk_data_->createImageViews();
  vk_data_->createRenderPass();
  vk_data_->createDescriptorSetLayout();
  vk_data_->createDescriptorPool();
  vk_data_->createGraphicsPipeline();
  vk_data_->createFramebuffers();
  vk_data_->createCommandPool();
  vk_data_->createTextureImage(); // TODO: remove
  vk_data_->createVertexBuffer();
  vk_data_->createIndexBuffer();
  vk_data_->createUniformBuffers();
  vk_data_->createCommandBuffers();
  vk_data_->createDescriptorSets();
  vk_data_->createSyncObjects();
}

void VkWrapper::drawFrame() { vk_data_->drawFrame(); }

} // namespace render
} // Namespace eldr
