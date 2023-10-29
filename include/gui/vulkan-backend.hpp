#pragma once
/* TODO: Migrate to vulkan.hpp and create automatic resource management */

#include <imgui.h> // ImVector
#include <string.h>
#include <vector>
#include <vulkan/vulkan.h>
////#include <memory>

// #define GLFW_INCLUDE_VULKAN

namespace eldr {
//
// Structs
//
struct VulkanFrame {
  VkCommandPool   command_pool;
  VkCommandBuffer command_buffer;
  VkFence         fence;
  VkImage         backbuffer;
  VkImageView     backbuffer_view;

  VkFramebuffer framebuffer;
};

struct VulkanFrameSemaphores {
  VkSemaphore image_acquired_semaphore;
  VkSemaphore render_complete_semaphore;
};

struct VulkanWindow {
  int                width;
  int                height;
  VkSwapchainKHR     swapchain;
  VkSurfaceKHR       surface;
  VkSurfaceFormatKHR surface_format;
  VkPresentModeKHR   present_mode;
  VkRenderPass       render_pass;
  bool               useDynamic_rendering;
  bool               clear_enable;
  VkClearValue       clear_value;
  // Current frame being rendered to (0 <= FrameIndex < FrameInFlightCount)
  uint32_t frame_index;
  // Number of simultaneous in-flight frames (returned by
  // vkGetSwapchainImagesKHR, usually derived from min_image_count)
  uint32_t image_count;
  // Current set of swapchain wait semaphores we're using (needs to be
  // distinct from per frame data)
  uint32_t semaphore_index;
  // The window pipeline may uses a different VkRenderPass than the one passed
  // in ImGui_ImplVulkan_InitInfo
  VkPipeline             pipeline;
  VulkanFrame*           frames;
  VulkanFrameSemaphores* frame_semaphores;

  VulkanWindow()
  {
    memset((void*) this, 0, sizeof(*this));
    present_mode = (VkPresentModeKHR) ~0; // Ensure we get an error if user
                                          // doesn't set this.
    clear_enable = true;
  }
};

struct VulkanData {
  VkAllocationCallbacks*   g_allocator;
  VkInstance               g_instance;
  VkPhysicalDevice         g_physical_device;
  VkDevice                 g_device;
  uint32_t                 g_queue_family;
  VkQueue                  g_queue;
  VkDebugUtilsMessengerEXT g_debug_messenger;
  VkPipelineCache          g_pipeline_cache;
  VkDescriptorPool         g_descriptor_pool;

  int  g_min_image_count;
  bool g_swap_chain_rebuild;
};

// TODO: Create vulkan wrapper probably

//
// Functions
//
void checkVkResult(VkResult);
int  initVulkan(VulkanData&, std::vector<const char*>&);
// bool isAvailableVkExtension(const ImVector<VkExtensionProperties>& props,
//                             const char* extension);
// void setupVulkanWindow(VulkanData vk_data, VulkanWindow* window,
//                        VkSurfaceKHR surface, int width, int height);
} // Namespace eldr
