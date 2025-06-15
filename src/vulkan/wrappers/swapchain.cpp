#include <eldr/core/platform.hpp>
#include <eldr/vulkan/wrappers/device.hpp>
#include <eldr/vulkan/wrappers/image.hpp>
#include <eldr/vulkan/wrappers/semaphore.hpp>
#include <eldr/vulkan/wrappers/surface.hpp>
#include <eldr/vulkan/wrappers/swapchain.hpp>

#include <GLFW/glfw3.h>

NAMESPACE_BEGIN(eldr::vk::wr)
// -----------------------------------------------------------------------------
// Helpers
// -----------------------------------------------------------------------------
NAMESPACE_BEGIN()
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
    extent.width  = std::clamp(requested_extent.width,
                              capabilities.minImageExtent.width,
                              capabilities.maxImageExtent.width);
    extent.height = std::clamp(requested_extent.height,
                               capabilities.minImageExtent.height,
                               capabilities.maxImageExtent.height);
    return extent;
  }
}
NAMESPACE_END()

// -----------------------------------------------------------------------------
// Swapchain
// -----------------------------------------------------------------------------
EL_VK_IMPL_DEFAULTS(Swapchain)

Swapchain::~Swapchain()
{
  if (vk()) {
    vkDestroySwapchainKHR(device().logical(), object_, nullptr);
  }
}

Swapchain::Swapchain(std::string_view name,
                     const Device&    device,
                     const Surface&   surface,
                     VkExtent2D       extent)
  : Base(name, device)
{
  setupSwapchain(device, surface, extent);
  // Create sync objects
  for (uint8_t i = 0; i < max_frames_in_flight; ++i) {
    image_available_sem_.emplace_back("Image available (swapchain)", device);
    render_finished_sem_.emplace_back("Render finished (swapchain)", device);
  }
}

void Swapchain::setupSwapchain(const Device&  device,
                               const Surface& surface,
                               VkExtent2D     requested_extent)
{
  const SwapchainSupportDetails& support_details{
    device.swapchainSupportDetails(surface.vk())
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

  VkSwapchainKHR old_swapchain{ object_ };

  VkSwapchainCreateInfoKHR swapchain_ci{
    .sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
    .pNext            = {},
    .flags            = {},
    .surface          = surface.vk(),
    .minImageCount    = min_image_count,
    .imageFormat      = surface_format_.format,
    .imageColorSpace  = surface_format_.colorSpace,
    .imageExtent      = extent_,
    .imageArrayLayers = 1,
    .imageUsage =
      VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
    .imageSharingMode      = {},
    .queueFamilyIndexCount = {},
    .pQueueFamilyIndices   = {},
    .preTransform          = support_details.capabilities.currentTransform,
    .compositeAlpha        = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
    .presentMode           = present_mode_,
    .clipped      = VK_TRUE, // don't care about color of obscured pixels
    .oldSwapchain = old_swapchain,
  };

  const QueueFamilyIndices& indices{ device.queueFamilyIndices() };
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

  Log(Trace, "Creating swapchain...");
  if (const VkResult result{ vkCreateSwapchainKHR(
        device.logical(), &swapchain_ci, nullptr, &object_) };
      result != VK_SUCCESS) {
    Throw("Failed to create swapchain! ({})", result);
  }

  // Destroy old swapchain
  if (old_swapchain) {
    vkDestroySwapchainKHR(device.logical(), old_swapchain, nullptr);
  }

  // Clear data from old swapchain
  images_.clear();
  std::vector<VkImage> images;

  // Get new swapchain images
  uint32_t image_count;
  if (const VkResult result{ vkGetSwapchainImagesKHR(
        device.logical(), object_, &image_count, nullptr) };
      result != VK_SUCCESS)
    Throw("Failed to get swapchain images! ({})", result);

  images.resize(image_count);

  if (const VkResult result{ vkGetSwapchainImagesKHR(
        device.logical(), object_, &image_count, images.data()) };
      result != VK_SUCCESS)
    Throw("Failed to get swapchain images! ({})", result);

  for (size_t i{ 0 }; i < images.size(); ++i) {
    images_.push_back(
      Image::createSwapchainImage(fmt::format("Swapchain image {}", i),
                                  device,
                                  images[i],
                                  extent_,
                                  surface_format_.format));
  }
}

const VkSemaphore* Swapchain::imageAvailableSemaphore(uint32_t index) const
{
  return &image_available_sem_[index].vk();
}

const VkSemaphore* Swapchain::renderFinishedSemaphore(uint32_t index) const
{
  return &render_finished_sem_[index].vk();
}

uint32_t Swapchain::acquireNextImage(uint32_t frame_index,
                                     bool&    invalidate_swapchain) const
{
  uint32_t       image_index{ 0 };
  const VkResult result{ vkAcquireNextImageKHR(
    device().logical(),
    object_,
    UINT64_MAX,
    image_available_sem_[frame_index].vk(),
    VK_NULL_HANDLE,
    &image_index) };

  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    invalidate_swapchain = true;
  }
  else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    Throw("Failed to acquire next image! ({})", result);

  return image_index;
}

void Swapchain::present(const VkPresentInfoKHR& present_info,
                        bool&                   invalidate_swapchain) const
{
  const VkResult result{ vkQueuePresentKHR(device().presentQueue(),
                                           &present_info) };
  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
    invalidate_swapchain = true;
  }
  else if (result != VK_SUCCESS) {
    Throw("Failed to present queue! ({})", result);
  }
}

const Image& Swapchain::image(size_t index) const { return images_[index]; }
Image&       Swapchain::image(size_t index) { return images_[index]; }

NAMESPACE_END(eldr::vk::wr)
