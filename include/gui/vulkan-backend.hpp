#pragma once

#include <cstdint>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#ifndef GLFW_INCLUDE_VULKAN
#define GLFW_INCLUDE_VULKAN
#endif

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <cstring> // TODO: remove if not using memset
#include <optional>
#include <string>
#include <vector>

namespace eldr {
namespace vk_wrapper {

struct VkWrapperInitInfo {
  GLFWwindow*               window;
  int                       width;
  int                       height;
  std::vector<const char*>& instance_extensions;
};

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

class Frame {
public:
  Frame() { memset((void*) this, VK_NULL_HANDLE, sizeof(*this)); }

private:
  VkCommandPool   command_pool_;
  VkCommandBuffer command_buffer_;
  VkFence         fence_;
  VkImage         backbuffer_;
  VkImageView     backbuffer_view_;
  VkFramebuffer   framebuffer_;
};

struct VulkanFrameSemaphores {
  VkSemaphore image_acquired_semaphore;
  VkSemaphore render_complete_semaphore;
};

class VkWrapper {
public:
  VkWrapper()
  {
    allocator_            = nullptr;
    instance_             = VK_NULL_HANDLE;
    physical_device_      = VK_NULL_HANDLE;
    device_               = VK_NULL_HANDLE;
    present_queue_        = VK_NULL_HANDLE;
    graphics_queue_       = VK_NULL_HANDLE;
    debug_messenger_      = VK_NULL_HANDLE;
    pipeline_cache_       = VK_NULL_HANDLE;
    descriptor_pool_      = VK_NULL_HANDLE;
    width_                = 0;
    height_               = 0;
    swapchain_            = VK_NULL_HANDLE;
    surface_              = VK_NULL_HANDLE;
    render_pass_          = VK_NULL_HANDLE;
    //useDynamic_rendering_ = false;
    //clear_enable_         = false;
    graphics_pipeline_             = VK_NULL_HANDLE;
    frames_               = nullptr;
    frame_semaphores_     = nullptr;
    min_image_count_      = 2;
    swapchain_rebuild_    = false;
    // surface_format_       = VK_NULL_HANDLE;
    // present_mode_         = VK_NULL_HANDLE;
    // clear_value_          = VK_NULL_HANDLE;
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
  int                      width_;  //!
  int                      height_; //!
  VkSwapchainKHR           swapchain_;
  std::vector<VkImage>     swapchain_images_;
  std::vector<VkImageView> swapchain_image_views_;
  VkFormat                 swapchain_image_format_;
  VkExtent2D               swapchain_extent_;
  VkSurfaceKHR             surface_;
  VkSurfaceFormatKHR       surface_format_;
  VkPresentModeKHR         present_mode_; //!
  //bool                     useDynamic_rendering_;
  //bool                     clear_enable_;
  //VkClearValue             clear_value_;
  //uint32_t                 frame_index_;
  uint32_t                 image_count_;
  //uint32_t                 semaphore_index_;
  VkPipeline               graphics_pipeline_;
  VkRenderPass             render_pass_;
  VkPipelineLayout         pipeline_layout_;
  Frame*                   frames_;
  VulkanFrameSemaphores*   frame_semaphores_;
  uint32_t                 min_image_count_;
  bool                     swapchain_rebuild_;

  // FUNCTIONS
#ifdef ELDR_VULKAN_DEBUG_REPORT
  void setupDebugMessenger();
#endif
  void createInstance(std::vector<const char*>& instance_extensions);
  void createSurface(GLFWwindow* window);
  void selectPhysicalDevice(std::vector<const char*>& device_extensions);
  void createLogicalDevice();
  void createSwapChain(GLFWwindow* window);
  void createImageViews();
  void createRenderPass();
  void createGraphicsPipeline();
  // void createRenderPass();
  // void createFramebuffers();
  void createCommandPool();
};
// TODO: ?
void checkVkResult(VkResult);

} // Namespace vk_wrapper
} // Namespace eldr