#include <GLFW/glfw3.h>
#include <cstdint>
#include <gui/viewer.hpp>
#include <imgui.h>
#include <memory>
#include <spdlog/spdlog.h>
#include <stdio.h>
#include <string>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

namespace eldr {

Viewer::Viewer(int width, int height, std::string name)
  : width_{ width }, height_{ height }, window_name_{ name }
{
  init();
}

Viewer::~Viewer()
{
  glfwDestroyWindow(window_);
  glfwTerminate();
}

static void checkVkResult(VkResult result)
{
  if (result == 0)
    return;
  spdlog::error("[VULKAN] Error: VkResult = %d", result);
  if (result < 0)
    abort();
}

static void glfwErrorCallback(int error, const char* description)
{
  fprintf(stderr, "GLFW Error %d: %s", error, description);
}

// VULKAN - Set up according to ImGui example
static bool isAvailableVkExtension(const ImVector<VkExtensionProperties>& props,
                                   const char* extension)
{
  for (const VkExtensionProperties& p : props)
    if (strcmp(p.extensionName, extension) == 0)
      return true;
  return false;
}

int Viewer::initVulkan(ImVector<const char*> instance_extensions)
{
  VkResult err;
  {
    // Create Vulkan instance
    VkInstanceCreateInfo create_info = {};
    create_info.sType                = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

    uint32_t properties_count;
    ImVector<VkExtensionProperties> properties;
    vkEnumerateInstanceExtensionProperties(nullptr, &properties_count, nullptr);
    properties.resize(properties_count);
    err = vkEnumerateInstanceExtensionProperties(nullptr, &properties_count,
                                                 properties.Data);
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
#ifdef IMGUI_VULKAN_DEBUG_REPORT
    const char* layers[]            = { "VK_LAYER_KHRONOS_validation" };
    create_info.enabledLayerCount   = 1;
    create_info.ppEnabledLayerNames = layers;
    instance_extensions.push_back("VK_EXT_DEBUG_REPORT");
#endif

    // Create Vulkan instance
    create_info.enabledExtensionCount   = (uint32_t) instance_extensions.Size;
    create_info.ppEnabledExtensionNames = instance_extensions.Data;
    err = vkCreateInstance(&create_info, vulkan_data_.g_allocator,
                           &vulkan_data_.g_instance);
    checkVkResult(err);

    //TODO: finish
    //
  } // end scope
  return 0;
}

int Viewer::init()
{
  glfwSetErrorCallback(glfwErrorCallback);
  if (!glfwInit()) {
    spdlog::error("GLFW: failed to initialize");
    return 1;
  }
  if (!glfwVulkanSupported()) {
    spdlog::error("GLFW: Vulkan not supported");
    return 1;
  }

  // Pre-init Vulkan data
  vulkan_data_.g_allocator        = nullptr;
  vulkan_data_.g_instance         = VK_NULL_HANDLE;
  vulkan_data_.g_physicalDevice   = VK_NULL_HANDLE;
  vulkan_data_.g_device           = VK_NULL_HANDLE;
  vulkan_data_.g_queueFamily      = (uint32_t) -1;
  vulkan_data_.g_queue            = VK_NULL_HANDLE;
  vulkan_data_.g_debugReport      = VK_NULL_HANDLE;
  vulkan_data_.g_pipelineCache    = VK_NULL_HANDLE;
  vulkan_data_.g_descriptorPool   = VK_NULL_HANDLE;
  vulkan_data_.g_minImageCount    = 2;
  vulkan_data_.g_swapChainRebuild = false;

  // Create GLFW Window with Vulkan context
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
  window_ =
    glfwCreateWindow(width_, height_, window_name_.c_str(), nullptr, nullptr);

  ImVector<const char*> extensions;
  uint32_t extensions_count = 0;
  const char** glfw_extensions =
    glfwGetRequiredInstanceExtensions(&extensions_count);
  for (size_t i = 0; i < extensions_count; i++)
    extensions.push_back(glfw_extensions[i]);

  // Initialize Vulkan
  return initVulkan(extensions);
}

} // Namespace eldr
