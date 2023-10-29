#include <GLFW/glfw3.h>
#include <core/util.hpp>
#include <cstdint>
#include <gui/gui.hpp>
#include <gui/vulkan-backend.hpp>
#include <imgui.h>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <stdio.h>
#include <string>
#include <vulkan/vulkan_core.h>

namespace eldr {

EldrGUI::EldrGUI(int width, int height, std::string name)
  : width_{ width }, height_{ height }, window_name_{ name }
{
  vulkan_data_.g_allocator          = nullptr;
  vulkan_data_.g_instance           = VK_NULL_HANDLE;
  vulkan_data_.g_physical_device    = VK_NULL_HANDLE;
  vulkan_data_.g_device             = VK_NULL_HANDLE;
  vulkan_data_.g_queue_family       = (uint32_t) -1;
  vulkan_data_.g_queue              = VK_NULL_HANDLE;
  vulkan_data_.g_debug_messenger    = VK_NULL_HANDLE;
  vulkan_data_.g_pipeline_cache     = VK_NULL_HANDLE;
  vulkan_data_.g_descriptor_pool    = VK_NULL_HANDLE;
  vulkan_data_.g_min_image_count    = 2;
  vulkan_data_.g_swap_chain_rebuild = false;

  init();
}

EldrGUI::~EldrGUI()
{
  VulkanData& vkd = vulkan_data_;
  // Destroy Vulkan objects
  vkDestroyDescriptorPool(vkd.g_device, vkd.g_descriptor_pool, vkd.g_allocator);
  vkDestroyDevice(vkd.g_device, vkd.g_allocator);
#ifdef ELDR_VULKAN_DEBUG_REPORT
  auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(
    vkd.g_instance, "vkDestroyDebugUtilsMessengerEXT");
  if (func != nullptr)
    func(vkd.g_instance, vkd.g_debug_messenger, vkd.g_allocator);
#endif
  // Instance should be destroyed last
  vkDestroyInstance(vkd.g_instance, vkd.g_allocator);

  glfwDestroyWindow(window_);
  glfwTerminate();
}

static void glfwErrorCallback(int error, const char* description)
{
  fprintf(stderr, "GLFW Error %d: %s", error, description);
}

void EldrGUI::init()
{
  // Initialize GLFW
  glfwSetErrorCallback(glfwErrorCallback);
  if (!glfwInit()) {
    throw std::runtime_error("[GLFW]: Failed to initialize");
  }

  // Vulkan pre-check
  if (!glfwVulkanSupported()) {
    throw std::runtime_error("[GLFW]: Vulkan not supported");
  }

  // Create GLFW Window with Vulkan context
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  window_ =
    glfwCreateWindow(width_, height_, window_name_.c_str(), nullptr, nullptr);

  std::vector<const char*> extensions;
  uint32_t                 extensions_count = 0;
  const char**             glfw_extensions =
    glfwGetRequiredInstanceExtensions(&extensions_count);
  for (size_t i = 0; i < extensions_count; i++)
    extensions.push_back(glfw_extensions[i]);

  // Initialize Vulkan
  initVulkan(vulkan_data_, extensions);
}

} // Namespace eldr
