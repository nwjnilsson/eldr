#include <eldr/vulkan/wrappers/device.hpp>
#include <eldr/vulkan/wrappers/image.hpp>
#include <eldr/vulkan/wrappers/semaphore.hpp>
#include <eldr/vulkan/wrappers/surface.hpp>
#include <eldr/vulkan/wrappers/swapchain.hpp>

#include <GLFW/glfw3.h>

namespace eldr::vk::wr {
// -----------------------------------------------------------------------------
// Helpers
// -----------------------------------------------------------------------------
namespace {
VkSurfaceFormatKHR selectSwapSurfaceFormat(
  const std::vector<VkSurfaceFormatKHR>& available_formats)
{
  for (const auto& a_format : available_formats) {
    if (a_format.format == VK_FORMAT_B8G8R8A8_SRGB &&
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

VkExtent2D selectSwapExtent(VkExtent2D                      requested_extent,
                            const VkSurfaceCapabilitiesKHR& capabilities)
{
  VkExtent2D extent;
  // Match the resolution of the current window, unless the value is the max
  // of uint32_t
  if (capabilities.currentExtent.width !=
      std::numeric_limits<uint32_t>::max()) {
    return capabilities.currentExtent;
  }
  else {
    extent.width =
      std::clamp(requested_extent.width, capabilities.minImageExtent.width,
                 capabilities.maxImageExtent.width);
    extent.height =
      std::clamp(requested_extent.height, capabilities.minImageExtent.height,
                 capabilities.maxImageExtent.height);
    return extent;
  }
}
} // namespace

// -----------------------------------------------------------------------------
// Swapchain
// -----------------------------------------------------------------------------
Swapchain::Swapchain(const Device& device, const Surface& surface,
                     VkExtent2D extent)
  : device_(device), surface_(surface)
{
  setupSwapchain(extent);
  // Create sync objects
  for (uint8_t i = 0; i < max_frames_in_flight; ++i) {
    image_available_sem_.emplace_back(device_);
    render_finished_sem_.emplace_back(device_);
  }
}

Swapchain::~Swapchain()
{
  if (swapchain_ != VK_NULL_HANDLE)
    vkDestroySwapchainKHR(device_.logical(), swapchain_, nullptr);

  while (!image_views_.empty()) {
    vkDestroyImageView(device_.logical(), image_views_.back(), nullptr);
    image_views_.pop_back();
  }
}

void Swapchain::setupSwapchain(VkExtent2D requested_extent)
{
  const SwapchainSupportDetails& support_details{
    device_.swapchainSupportDetails(surface_.get())
  };
  extent_ = selectSwapExtent(requested_extent, support_details.capabilities);
  surface_format_ = selectSwapSurfaceFormat(support_details.formats);
  present_mode_   = selectSwapPresentMode(support_details.present_modes);
  //----------------------------------------------------------------------------
  // Create swapchain
  //----------------------------------------------------------------------------
  uint32_t min_image_count{ support_details.capabilities.minImageCount + 1 };
  // If there is an upper limit, make sure we don't exceed it
  if (support_details.capabilities.maxImageCount > 0) {
    min_image_count =
      std::min(min_image_count, support_details.capabilities.maxImageCount);
  }

  const VkSwapchainKHR     old_swapchain{ swapchain_ };
  VkSwapchainCreateInfoKHR swapchain_ci{
    .sType                 = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
    .pNext                 = {},
    .flags                 = {},
    .surface               = surface_.get(),
    .minImageCount         = min_image_count,
    .imageFormat           = surface_format_.format,
    .imageColorSpace       = surface_format_.colorSpace,
    .imageExtent           = extent_,
    .imageArrayLayers      = 1,
    .imageUsage            = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
    .imageSharingMode      = {},
    .queueFamilyIndexCount = {},
    .pQueueFamilyIndices   = {},
    .preTransform          = support_details.capabilities.currentTransform,
    .compositeAlpha        = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
    .presentMode           = present_mode_,
    .clipped      = VK_TRUE, // don't care about color of obscured pixels
    .oldSwapchain = old_swapchain,
  };

  const QueueFamilyIndices& indices{ device_.queueFamilyIndices() };
  uint32_t queueFamilyIndices[]{ indices.graphics_family.value(),
                                 indices.present_family.value() };

  if (indices.graphics_family != indices.present_family) {
    swapchain_ci.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
    swapchain_ci.queueFamilyIndexCount = 2;
    swapchain_ci.pQueueFamilyIndices   = queueFamilyIndices;
  }
  else {
    swapchain_ci.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_ci.queueFamilyIndexCount = 0;       // Optional
    swapchain_ci.pQueueFamilyIndices   = nullptr; // Optional
  }

  core::requestLogger("vulkan-engine")->trace("Creating swapchain...");
  if (const VkResult result = vkCreateSwapchainKHR(
        device_.logical(), &swapchain_ci, nullptr, &swapchain_);
      result != VK_SUCCESS) {
    ThrowVk(result, "vkCreateSwapchainKHR(): ");
  }

  // Destroy old swapchain if it exists
  if (old_swapchain != VK_NULL_HANDLE) {
    while (!image_views_.empty()) {
      vkDestroyImageView(device_.logical(), image_views_.back(), nullptr);
      image_views_.pop_back();
    }
    images_.clear();
    vkDestroySwapchainKHR(device_.logical(), old_swapchain, nullptr);
  }

  // Get swapchain images
  uint32_t image_count;
  if (const auto result = vkGetSwapchainImagesKHR(device_.logical(), swapchain_,
                                                  &image_count, nullptr);
      result != VK_SUCCESS)
    ThrowVk(result, "vkGetSwapchainImagesKHR(): ");

  images_.resize(image_count);

  if (const auto result = vkGetSwapchainImagesKHR(device_.logical(), swapchain_,
                                                  &image_count, images_.data());
      result != VK_SUCCESS)
    ThrowVk(result, "vkGetSwapchainImagesKHR(): ");

  //----------------------------------------------------------------------------
  // Create image views
  //----------------------------------------------------------------------------
  VkImageViewCreateInfo image_view_ci{
    .sType            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
    .pNext            = {},
    .flags            = {},
    .image            = {},
    .viewType         = VK_IMAGE_VIEW_TYPE_2D,
    .format           = surface_format_.format,
    .components       = { VK_COMPONENT_SWIZZLE_IDENTITY,
                          VK_COMPONENT_SWIZZLE_IDENTITY,
                          VK_COMPONENT_SWIZZLE_IDENTITY,
                          VK_COMPONENT_SWIZZLE_IDENTITY },
    .subresourceRange = { .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
                          .baseMipLevel   = 0,
                          .levelCount     = 1,
                          .baseArrayLayer = 0,
                          .layerCount     = 1 }
  };
  image_views_.resize(image_count);
  for (size_t i = 0; i < images_.size(); i++) {
    image_view_ci.image = images_[i];
    device_.createImageView(image_view_ci, &image_views_[i],
                            "swapchain image view");
  }
}

const VkSemaphore* Swapchain::imageAvailableSemaphore(uint32_t index) const
{
  return image_available_sem_[index].ptr();
}

const VkSemaphore* Swapchain::renderFinishedSemaphore(uint32_t index) const
{
  return render_finished_sem_[index].ptr();
}

uint32_t Swapchain::acquireNextImage(uint32_t frame_index,
                                     bool&    invalidate_swapchain)
{
  uint32_t   image_index{ 0 };
  const auto result{ vkAcquireNextImageKHR(
    device_.logical(), swapchain_, UINT64_MAX,
    image_available_sem_[frame_index].get(), VK_NULL_HANDLE, &image_index) };

  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    invalidate_swapchain = true;
  }
  else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    ThrowVk(result, "vkAcquireNextImageKHR(): ");

  return image_index;
}

void Swapchain::present(const VkPresentInfoKHR& present_info,
                        bool&                   invalidate_swapchain)
{
  const auto result = vkQueuePresentKHR(device_.presentQueue(), &present_info);
  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
    invalidate_swapchain = true;
  }
  else if (result != VK_SUCCESS) {
    ThrowVk(result, "vkQueuePresentKHR(): ");
  }
}

} // namespace eldr::vk::wr
