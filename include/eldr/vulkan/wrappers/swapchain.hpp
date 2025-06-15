#pragma once
#include <eldr/vulkan/vulkan.hpp>
#include <eldr/vulkan/wrappers/semaphore.hpp>

#include <vector>

NAMESPACE_BEGIN(eldr::vk::wr)
class Swapchain : public VkDeviceObject<VkSwapchainKHR> {
  using Base = VkDeviceObject<VkSwapchainKHR>;

public:
  EL_VK_IMPORT_DEFAULTS(Swapchain)
  Swapchain(std::string_view name, const Device&, const Surface&, VkExtent2D);

  [[nodiscard]] const VkExtent2D& extent() const { return extent_; }
  //[[nodiscard]] uint32_t minImageCount() const { return min_image_count_; }
  [[nodiscard]] const Image& image(size_t index) const;
  [[nodiscard]] Image&       image(size_t index);
  [[nodiscard]] VkFormat imageFormat() const { return surface_format_.format; }
  [[nodiscard]] const VkSemaphore*
  imageAvailableSemaphore(uint32_t index) const;
  [[nodiscard]] const VkSemaphore*
  renderFinishedSemaphore(uint32_t index) const;

  /// Create/recreate the swapchain
  /// @param extent The swapchain extent
  void setupSwapchain(const Device&  device,
                      const Surface& surface,
                      VkExtent2D     extent);

  [[nodiscard]] uint32_t acquireNextImage(uint32_t frame_index,
                                          bool&    invalidate_swapchain) const;

  void present(const VkPresentInfoKHR& present_info,
               bool&                   invalidate_swapchain) const;

private:
  VkExtent2D         extent_;
  VkSurfaceFormatKHR surface_format_;
  VkPresentModeKHR   present_mode_;

  std::vector<Image>     images_;
  std::vector<Semaphore> image_available_sem_;
  std::vector<Semaphore> render_finished_sem_;
};
NAMESPACE_END(eldr::vk::wr)
