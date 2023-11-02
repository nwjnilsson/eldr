#pragma once

#include <cstdint>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#ifndef GLFW_INCLUDE_VULKAN
#define GLFW_INCLUDE_VULKAN
#endif

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <imgui.h> // ImVector
#include <optional>
#include <string>
#include <vector>

namespace eldr {
namespace vk_wrapper {

struct VkWrapperInitInfo {
  GLFWwindow*               window;
  int                       width;
  int                       height;
  std::vector<const char*>& extensions;
};

struct QueueFamilyIndices {
  std::optional<uint32_t> graphics_family;
  std::optional<uint32_t> present_family;
  bool                    isComplete()
  {
    return graphics_family.has_value() && present_family.has_value();
  }
};

class Frame {
public:
  Frame() { memset((void*) this, VK_NULL_HANDLE, sizeof(*this)); }

private:
  // FRAME RELATED
  VkCommandPool   command_pool_;
  VkCommandBuffer command_buffer_;
  VkFence         fence_;
  VkImage         backbuffer_;
  VkImageView     backbuffer_view_;

  VkFramebuffer framebuffer_;
};

struct VulkanFrameSemaphores {
  VkSemaphore image_acquired_semaphore;
  VkSemaphore render_complete_semaphore;
};

class VkWrapper {
public:
  VkWrapper()
  {

    // TODO Implement
    // Some variables will be VK_NULL_HANDLE while indices will need to be
    // initialized to -1
  }

  void init(VkWrapperInitInfo&);
  void destroy();

private:
  // DEVICE RELATED
  VkAllocationCallbacks*   allocator_;
  VkInstance               instance_;
  VkPhysicalDevice         physical_device_;
  VkDevice                 device_;
  VkQueue                  present_queue_;
  VkQueue                  graphics_queue_;
  VkDebugUtilsMessengerEXT debug_messenger_;
  VkPipelineCache          pipeline_cache_;
  VkDescriptorPool         descriptor_pool_;

  // WINDOW RELATED
  int                width_;
  int                height_;
  VkSwapchainKHR     swapchain_;
  VkSurfaceKHR       surface_;
  VkSurfaceFormatKHR surface_format_;
  VkPresentModeKHR   present_mode_;
  VkRenderPass       render_pass_;
  bool               useDynamic_rendering_;
  bool               clear_enable_;
  VkClearValue       clear_value_;
  // Current frame being rendered to (0 <= FrameIndex < FrameInFlightCount)
  uint32_t frame_index_;
  // Number of simultaneous in-flight frames (returned by
  // vkGetSwapchainImagesKHR, usually derived from min_image_count)
  uint32_t image_count_;
  // Current set of swapchain wait semaphores we're using (needs to be
  // distinct from per frame data)
  uint32_t semaphore_index_;
  // The window pipeline may uses a different VkRenderPass than the one passed
  // in ImGui_ImplVulkan_InitInfo
  VkPipeline             pipeline_;
  Frame*                 frames_;
  VulkanFrameSemaphores* frame_semaphores_;
  int                    min_image_count_;
  bool                   swap_chain_rebuild_;

  // FUNCTIONS
  // Device
#ifdef ELDR_VULKAN_DEBUG_REPORT
  void setupDebugMessenger();
#endif
  void createInstance(std::vector<const char*>& instance_extensions);
  void createSurface();
  void selectPhysicalDevice();
  void createLogicalDevice();
  // void createSwapChain();
  // void createImageViews();
  // void createRenderPass();
  // void createGraphicsPipeline();
  // void createFramebuffers();
  void createCommandPool();

  // Window
  void setupVulkanWindow(GLFWwindow*, int w, int h);
};

void checkVkResult(VkResult);
} // Namespace vk_wrapper
} // Namespace eldr
