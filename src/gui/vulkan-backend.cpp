#include <core/util.hpp>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <gui/vulkan-backend.hpp>
#include <iostream>
#include <limits>
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
  SwapChainSupportDetails swapchain_support =
    swapChainSupportDetails(device, surface);

  return required_extensions.empty() && indices.isComplete() &&
         !swapchain_support.formats.empty() &&
         !swapchain_support.present_modes.empty();
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
  // overall score, and when the properties of all devices have been
  // evaluated, the highest scoring device is considered the most suitable
  // one. For now I don't really think it matters. Just use a discrete GPU if
  // it's available.
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

void VkWrapper::createSwapChain(GLFWwindow* window)
{
  SwapChainSupportDetails swapchain_support =
    swapChainSupportDetails(physical_device_, surface_);
  surface_format_   = selectSwapSurfaceFormat(swapchain_support.formats);
  present_mode_     = selectSwapPresentMode(swapchain_support.present_modes);
  VkExtent2D extent = selectSwapExtent(window, swapchain_support.capabilities);

  // TODO: keep this image count?
  min_image_count_ = swapchain_support.capabilities.minImageCount + 1;
  // If there is an upper limit, make sure we don't exceed it
  if (swapchain_support.capabilities.maxImageCount > 0) {
    min_image_count_ =
      std::min(min_image_count_, swapchain_support.capabilities.maxImageCount);
  }
  VkSwapchainCreateInfoKHR swap_ci{};
  swap_ci.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  swap_ci.surface          = surface_;
  swap_ci.minImageCount    = min_image_count_;
  swap_ci.imageFormat      = surface_format_.format;
  swap_ci.imageColorSpace  = surface_format_.colorSpace;
  swap_ci.imageExtent      = extent;
  swap_ci.imageArrayLayers = 1;
  swap_ci.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  swap_ci.preTransform     = swapchain_support.capabilities.currentTransform;
  swap_ci.compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  swap_ci.presentMode      = present_mode_;
  swap_ci.clipped      = VK_TRUE; // don't care about color of obscured pixels
  swap_ci.oldSwapchain = VK_NULL_HANDLE;

  QueueFamilyIndices indices = findQueueFamilies(physical_device_, surface_);
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

  if (vkCreateSwapchainKHR(device_, &swap_ci, allocator_, &swapchain_) !=
      VK_SUCCESS) {
    throwVkErr("Failed to create swapchain");
  }
  vkGetSwapchainImagesKHR(device_, swapchain_, &image_count_, nullptr);
  swapchain_images_.resize(image_count_);
  vkGetSwapchainImagesKHR(device_, swapchain_, &image_count_,
                          swapchain_images_.data());
  swapchain_extent_       = extent;
  swapchain_image_format_ = surface_format_.format;
}

void VkWrapper::createImageViews()
{
  swapchain_image_views_.resize(swapchain_images_.size());
  for (size_t i = 0; i < swapchain_images_.size(); i++) {
    VkImageViewCreateInfo image_view_ci{};
    image_view_ci.sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    image_view_ci.image    = swapchain_images_[i];
    image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_ci.format   = swapchain_image_format_;
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

    if (vkCreateImageView(device_, &image_view_ci, allocator_,
                          &swapchain_image_views_[i]) != VK_SUCCESS) {
      throwVkErr("Failed to create image view!");
    }
  }
}

void VkWrapper::createRenderPass()
{
  VkAttachmentDescription color_attachment{};
  color_attachment.format         = swapchain_image_format_;
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

  if (vkCreateRenderPass(device_, &render_pass_ci, allocator_, &render_pass_) !=
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

void VkWrapper::createGraphicsPipeline()
{
  std::vector<char> vert_shader = loadShader("vertex");
  std::vector<char> frag_shader = loadShader("fragment");
  spdlog::info("Vertex shader size = {}", vert_shader.size());
  spdlog::info("Fragment shader size = {}", frag_shader.size());

  VkShaderModule vert_shader_module =
    createShaderModule(device_, allocator_, vert_shader);
  VkShaderModule frag_shader_module =
    createShaderModule(device_, allocator_, frag_shader);

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
  vertex_input_info.sType =
    VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertex_input_info.vertexBindingDescriptionCount   = 0;
  vertex_input_info.pVertexBindingDescriptions      = nullptr; // Optional
  vertex_input_info.vertexAttributeDescriptionCount = 0;
  vertex_input_info.pVertexAttributeDescriptions    = nullptr; // Optional

  VkPipelineInputAssemblyStateCreateInfo input_assembly{};
  input_assembly.sType =
    VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  // TODO: Use different topology?
  input_assembly.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  input_assembly.primitiveRestartEnable = VK_FALSE;

  VkViewport viewport{};
  viewport.x        = 0.0f;
  viewport.y        = 0.0f;
  viewport.width    = (float) swapchain_extent_.width;
  viewport.height   = (float) swapchain_extent_.height;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  VkRect2D scissor{};
  scissor.offset = { 0, 0 };
  scissor.extent = swapchain_extent_;

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
  rasterizer.frontFace               = VK_FRONT_FACE_CLOCKWISE;
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
  pipeline_layout_ci.setLayoutCount         = 0;       // Optional
  pipeline_layout_ci.pSetLayouts            = nullptr; // Optional
  pipeline_layout_ci.pushConstantRangeCount = 0;       // Optional
  pipeline_layout_ci.pPushConstantRanges    = nullptr; // Optional

  if (vkCreatePipelineLayout(device_, &pipeline_layout_ci, nullptr,
                             &pipeline_layout_) != VK_SUCCESS) {
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
  pipeline_ci.layout              = pipeline_layout_;
  pipeline_ci.renderPass          = render_pass_;
  pipeline_ci.subpass             = 0;
  pipeline_ci.basePipelineHandle  = VK_NULL_HANDLE; // Optional
  pipeline_ci.basePipelineIndex   = -1;             // Optional

  if (vkCreateGraphicsPipelines(device_, VK_NULL_HANDLE, 1, &pipeline_ci,
                                nullptr, &graphics_pipeline_) != VK_SUCCESS) {
    throwVkErr("Failed to create graphics pipeline!");
  }

  vkDestroyShaderModule(device_, vert_shader_module, allocator_);
  vkDestroyShaderModule(device_, frag_shader_module, allocator_);
}

void VkWrapper::createFramebuffers()
{
  swapchain_framebuffers_.resize(swapchain_image_views_.size());
  for (size_t i = 0; i < swapchain_image_views_.size(); ++i) {
    VkImageView             attachments[] = { swapchain_image_views_[i] };
    VkFramebufferCreateInfo framebuffer_ci{};
    framebuffer_ci.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebuffer_ci.renderPass      = render_pass_;
    framebuffer_ci.attachmentCount = 1;
    framebuffer_ci.pAttachments    = attachments;
    framebuffer_ci.width           = swapchain_extent_.width;
    framebuffer_ci.height          = swapchain_extent_.height;
    framebuffer_ci.layers          = 1;

    if (vkCreateFramebuffer(device_, &framebuffer_ci, allocator_,
                            &swapchain_framebuffers_[i]) != VK_SUCCESS)
      throwVkErr("Failed to create framebuffer!");
  }
}

void VkWrapper::createCommandPool()
{
  QueueFamilyIndices queue_family_indices =
    findQueueFamilies(physical_device_, surface_);

  VkCommandPoolCreateInfo pool_ci = {};
  pool_ci.sType                   = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  pool_ci.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  pool_ci.queueFamilyIndex = queue_family_indices.graphics_family.value();
  if (vkCreateCommandPool(device_, &pool_ci, allocator_, &command_pool_) !=
      VK_SUCCESS)
    throwVkErr("Failed to create command pool!");
}

void VkWrapper::createCommandBuffer()
{
  VkCommandBufferAllocateInfo alloc_info{};
  alloc_info.sType       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  alloc_info.commandPool = command_pool_;
  alloc_info.level       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  alloc_info.commandBufferCount = 1;

  if (vkAllocateCommandBuffers(device_, &alloc_info, &command_buffer_) !=
      VK_SUCCESS)
    throwVkErr("Failed to create command buffer!");
}

void VkWrapper::createSurface(GLFWwindow* window)
{
  // Create surface
  if (glfwCreateWindowSurface(instance_, window, allocator_, &surface_) !=
      VK_SUCCESS)
    throwVkErr("Failed to create window surface!");
}

void VkWrapper::createSyncObjects()
{
  VkSemaphoreCreateInfo semaphore_ci{};
  semaphore_ci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  VkFenceCreateInfo fence_ci{};
  fence_ci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fence_ci.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  if (vkCreateSemaphore(device_, &semaphore_ci, allocator_,
                        &image_available_sem_) != VK_SUCCESS ||
      vkCreateSemaphore(device_, &semaphore_ci, allocator_,
                        &render_finished_sem_) != VK_SUCCESS ||
      vkCreateFence(device_, &fence_ci, allocator_, &in_flight_fence_) !=
        VK_SUCCESS)
    throwVkErr("Failed to create sync objects!");
}

void VkWrapper::init(VkWrapperInitInfo& init_info)
{
  createInstance(init_info.instance_extensions);
#ifdef ELDR_VULKAN_DEBUG_REPORT
  setupDebugMessenger();
#endif
  createSurface(init_info.window);
  createLogicalDevice(); // Also selects physical device
  createSwapChain(init_info.window);
  createImageViews();
  createRenderPass();
  createGraphicsPipeline();
  createFramebuffers();
  createCommandPool();
  createCommandBuffer();
  createSyncObjects();
}

void VkWrapper::destroy()
{
  vkDeviceWaitIdle(device_);

  vkDestroySemaphore(device_, image_available_sem_, allocator_);
  vkDestroySemaphore(device_, render_finished_sem_, allocator_);
  vkDestroyFence(device_, in_flight_fence_, allocator_);
  vkDestroyCommandPool(device_, command_pool_, allocator_);

  for (auto& framebuffer : swapchain_framebuffers_)
    vkDestroyFramebuffer(device_, framebuffer, allocator_);

  vkDestroyPipeline(device_, graphics_pipeline_, allocator_);
  vkDestroyPipelineLayout(device_, pipeline_layout_, allocator_);
  vkDestroyRenderPass(device_, render_pass_, allocator_);

  for (auto& image_view : swapchain_image_views_)
    vkDestroyImageView(device_, image_view, allocator_);

  vkDestroySwapchainKHR(device_, swapchain_, allocator_);
  vkDestroyDescriptorPool(device_, descriptor_pool_, allocator_);
  vkDestroySurfaceKHR(instance_, surface_, allocator_);
  vkDestroyDevice(device_, allocator_);

#ifdef ELDR_VULKAN_DEBUG_REPORT
  auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(
    instance_, "vkDestroyDebugUtilsMessengerEXT");
  if (func != nullptr)
    func(instance_, debug_messenger_, allocator_);
#endif
  // Instance should be destroyed last
  vkDestroyInstance(instance_, allocator_);
}

void VkWrapper::recordCommandBuffer(uint32_t image_index)
{
  VkCommandBufferBeginInfo command_buffer_info{};
  command_buffer_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  command_buffer_info.flags = 0;
  command_buffer_info.pInheritanceInfo = nullptr;

  if (vkBeginCommandBuffer(command_buffer_, &command_buffer_info) != VK_SUCCESS)
    throwVkErr("Failed to create command buffer!");

  VkRenderPassBeginInfo render_pass_info{};
  render_pass_info.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  render_pass_info.renderPass        = render_pass_;
  render_pass_info.framebuffer       = swapchain_framebuffers_[image_index];
  render_pass_info.renderArea.offset = { 0, 0 };
  render_pass_info.renderArea.extent = swapchain_extent_;

  VkClearValue clear_color         = { { { 0.0f, 0.0f, 0.0f, 1.0f } } };
  render_pass_info.clearValueCount = 1;
  render_pass_info.pClearValues    = &clear_color;

  vkCmdBeginRenderPass(command_buffer_, &render_pass_info,
                       VK_SUBPASS_CONTENTS_INLINE);

  vkCmdBindPipeline(command_buffer_, VK_PIPELINE_BIND_POINT_GRAPHICS,
                    graphics_pipeline_);
  VkViewport viewport{};
  viewport.x        = 0.0f;
  viewport.y        = 0.0f;
  viewport.width    = static_cast<float>(swapchain_extent_.width);
  viewport.height   = static_cast<float>(swapchain_extent_.height);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  vkCmdSetViewport(command_buffer_, 0, 1, &viewport);

  VkRect2D scissor{};
  scissor.offset = { 0, 0 };
  scissor.extent = swapchain_extent_;
  vkCmdSetScissor(command_buffer_, 0, 1, &scissor);

  vkCmdDraw(command_buffer_, 3, 1, 0, 0);
  vkCmdEndRenderPass(command_buffer_);
  if (vkEndCommandBuffer(command_buffer_) != VK_SUCCESS)
    throwVkErr("Failed to record command buffer!");
}

void VkWrapper::drawFrame()
{
  vkWaitForFences(device_, 1, &in_flight_fence_, VK_TRUE, UINT64_MAX);
  vkResetFences(device_, 1, &in_flight_fence_);

  uint32_t image_index;
  vkAcquireNextImageKHR(device_, swapchain_, UINT64_MAX, image_available_sem_,
                        VK_NULL_HANDLE, &image_index);
  vkResetCommandBuffer(command_buffer_, 0);
  recordCommandBuffer(image_index);

  VkSubmitInfo submit_info{};
  submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

  VkSemaphore          wait_semaphores[] = { image_available_sem_ };
  VkPipelineStageFlags wait_stages[]     = {
    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
  };
  submit_info.waitSemaphoreCount   = 1;
  submit_info.pWaitSemaphores      = wait_semaphores;
  submit_info.pWaitDstStageMask    = wait_stages;
  submit_info.commandBufferCount   = 1;
  submit_info.pCommandBuffers      = &command_buffer_;
  VkSemaphore signal_semaphores[]  = { render_finished_sem_ };
  submit_info.signalSemaphoreCount = 1;
  submit_info.pSignalSemaphores    = signal_semaphores;

  if (vkQueueSubmit(graphics_queue_, 1, &submit_info, in_flight_fence_) !=
      VK_SUCCESS)
    throwVkErr("Failed to submit draw command buffer!");

  VkPresentInfoKHR present_info{};
  present_info.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  present_info.waitSemaphoreCount = 1;
  present_info.pWaitSemaphores    = signal_semaphores;

  VkSwapchainKHR swapchains[] = { swapchain_ };
  present_info.swapchainCount  = 1;
  present_info.pSwapchains     = swapchains;
  present_info.pImageIndices   = &image_index;

  vkQueuePresentKHR(present_queue_, &present_info);
}

} // Namespace vk_wrapper
} // Namespace eldr
