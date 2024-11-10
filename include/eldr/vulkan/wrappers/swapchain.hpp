#pragma once

#include <eldr/vulkan/common.hpp>

#include <vector>

namespace eldr::vk::wr {
class Swapchain {
public:
  Swapchain(const Device&, const Surface&, VkExtent2D);
  ~Swapchain();

  [[nodiscard]] const VkExtent2D& extent() const { return extent_; }
  [[nodiscard]] VkSwapchainKHR    get() const { return swapchain_; }
  //[[nodiscard]] uint32_t minImageCount() const { return min_image_count_; }
  [[nodiscard]] const std::vector<VkImageView>& imageViews() const
  {
    return image_views_;
  }
  [[nodiscard]] VkFormat imageFormat() const { return surface_format_.format; }
  [[nodiscard]] const VkSemaphore*
  imageAvailableSemaphore(uint32_t index) const;
  [[nodiscard]] const VkSemaphore*
  renderFinishedSemaphore(uint32_t index) const;

  /// Create/recreate the swapchain
  /// @param extent The swapchain extent
  void setupSwapchain(VkExtent2D extent);

  [[nodiscard]] uint32_t acquireNextImage(uint32_t frame_index);

  void present(const VkPresentInfoKHR& present_info);

private:
  const Device&  device_;
  const Surface& surface_;

  VkExtent2D         extent_;
  VkSurfaceFormatKHR surface_format_;
  VkPresentModeKHR   present_mode_;
  VkSwapchainKHR     swapchain_{ VK_NULL_HANDLE };

  std::vector<VkImage>     images_;
  std::vector<VkImageView> image_views_;
  std::vector<Semaphore>   image_available_sem_;
  std::vector<Semaphore>   render_finished_sem_;
};
} // namespace eldr::vk::wr
