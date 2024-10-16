#pragma once

#include <eldr/vulkan/common.hpp>
#include <eldr/vulkan/helpers.hpp>
#include <eldr/vulkan/wrappers/device.hpp>

#include <memory>
#include <vector>

// fwd
struct GLFWwindow;

namespace eldr::vk::wr {
class Swapchain {
public:
  Swapchain(const Device&, const Surface&, VkExtent2D,
            const SwapchainSupportDetails&);
  ~Swapchain();

  const VkExtent2D&     extent() const { return extent_; }
  VkSwapchainKHR        get() const { return swapchain_; }
  uint32_t              minImageCount() const { return min_image_count_; }
  VkSampleCountFlagBits msaaSamples() const { return msaa_samples_; }
  const std::vector<VkImageView>& imageViews() const { return image_views_; }

  void     recreate(Window&);
  VkResult acquireNextImage(uint32_t& index, Semaphore& image_available_sem);

private:
  void createSwapchain(const SwapchainSupportDetails&);
  void createFramebuffers();
  void createImageViews();
  void cleanup();

private:
  const Device&  device_;
  const Surface& surface_;

  VkExtent2D            extent_;
  VkSurfaceFormatKHR    surface_format_;
  VkSampleCountFlagBits msaa_samples_{ VK_SAMPLE_COUNT_1_BIT };
  VkPresentModeKHR      present_mode_;
  VkSwapchainKHR        swapchain_{ VK_NULL_HANDLE };

  uint32_t min_image_count_;
  uint32_t image_count_;

  std::unique_ptr<Image> color_image_;
  std::unique_ptr<Image> depth_image_;

  std::vector<VkImage>     images_;
  std::vector<VkImageView> image_views_;
};
} // namespace eldr::vk::wr
