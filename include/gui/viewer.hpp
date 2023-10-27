#pragma once

#include <GLFW/glfw3.h>
#include <memory>
#include <string.h>
#include <vulkan/vulkan.h>
#include <imgui.h>

#define GLFW_INCLUDE_VULKAN

namespace eldr {

struct VulkanFrame {
  VkCommandPool   CommandPool;
  VkCommandBuffer CommandBuffer;
  VkFence         Fence;
  VkImage         Backbuffer;
  VkImageView     BackbufferView;
  VkFramebuffer   Framebuffer;
};

struct VulkanFrameSemaphores {
  VkSemaphore ImageAcquiredSemaphore;
  VkSemaphore RenderCompleteSemaphore;
};

struct VulkanWindow {
  int                 Width;
  int                 Height;
  VkSwapchainKHR      Swapchain;
  VkSurfaceKHR        Surface;
  VkSurfaceFormatKHR  SurfaceFormat;
  VkPresentModeKHR    PresentMode;
  VkRenderPass        RenderPass;
  bool                UseDynamicRendering;
  bool                ClearEnable;
  VkClearValue        ClearValue;
  // Current frame being rendered to (0 <= FrameIndex < FrameInFlightCount)
  uint32_t FrameIndex;
  // Number of simultaneous in-flight frames (returned by
  // vkGetSwapchainImagesKHR, usually derived from min_image_count)
  uint32_t ImageCount;
  // Current set of swapchain wait semaphores we're using (needs to be distinct
  // from per frame data)
  uint32_t SemaphoreIndex;
  // The window pipeline may uses a different VkRenderPass than the one passed
  // in ImGui_ImplVulkan_InitInfo
  VkPipeline             Pipeline;
  VulkanFrame*           Frames;
  VulkanFrameSemaphores* FrameSemaphores;

  VulkanWindow()
  {
    memset((void*)this, 0, sizeof(*this));
    PresentMode = (VkPresentModeKHR)~0;     // Ensure we get an error if user doesn't set this.
    ClearEnable = true;
  }
};

struct VulkanData {
  VkAllocationCallbacks*   g_allocator;
  VkInstance               g_instance;
  VkPhysicalDevice         g_physicalDevice;
  VkDevice                 g_device;
  uint32_t                 g_queueFamily;
  VkQueue                  g_queue;
  VkDebugReportCallbackEXT g_debugReport;
  VkPipelineCache          g_pipelineCache;
  VkDescriptorPool         g_descriptorPool;

  VulkanWindow g_mainWindowData;
  int          g_minImageCount;
  bool         g_swapChainRebuild;
};

class Viewer {
  public:
    Viewer(int width, int height, std::string name);
    ~Viewer();

    //Viewer(const Viewer &)            = delete;
    //Viewer &operator=(const Viewer &) = delete;

    inline bool should_close() { return glfwWindowShouldClose(window_); }
    static int initVulkan(ImVector<const char*>);
    int init();

  private:
    const int width_;
    const int height_;
    std::string window_name_;

    static VulkanData vulkan_data_;
    GLFWwindow* window_;
};
} // namespace eldr
