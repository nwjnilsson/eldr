#include <eldr/core/platform.hpp>
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
    extent.width  = std::clamp(requested_extent.width,
                              capabilities.minImageExtent.width,
                              capabilities.maxImageExtent.width);
    extent.height = std::clamp(requested_extent.height,
                               capabilities.minImageExtent.height,
                               capabilities.maxImageExtent.height);
    return extent;
  }
}
} // namespace

//------------------------------------------------------------------------------
// SwapchainImpl
//------------------------------------------------------------------------------
class Swapchain::SwapchainImpl {
public:
  SwapchainImpl(const Device&                   device,
                const VkSwapchainCreateInfoKHR& swapchain_ci);
  ~SwapchainImpl();
  const Device   device_;
  VkSwapchainKHR swapchain_{ VK_NULL_HANDLE };
};

Swapchain::SwapchainImpl::SwapchainImpl(
  const Device& device, const VkSwapchainCreateInfoKHR& swapchain_ci)
  : device_(device)
{
  if (const VkResult result{ vkCreateSwapchainKHR(
        device_.logical(), &swapchain_ci, nullptr, &swapchain_) };
      result != VK_SUCCESS) {
    Throw("Failed to create swapchain! ({})", result);
  }
}

Swapchain::SwapchainImpl::~SwapchainImpl()
{
  vkDestroySwapchainKHR(device_.logical(), swapchain_, nullptr);
}

// -----------------------------------------------------------------------------
// Swapchain
// -----------------------------------------------------------------------------
Swapchain::Swapchain(const Device&  device,
                     const Surface& surface,
                     VkExtent2D     extent)
{
  setupSwapchain(device, surface, extent);
  // Create sync objects
  for (uint8_t i = 0; i < max_frames_in_flight; ++i) {
    image_available_sem_.emplace_back(device);
    render_finished_sem_.emplace_back(device);
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

  VkSwapchainKHR old_swapchain{ VK_NULL_HANDLE };
  if (likely(sc_data_ != nullptr))
    old_swapchain = sc_data_->swapchain_;
  VkSwapchainCreateInfoKHR swapchain_ci{
    .sType                 = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
    .pNext                 = {},
    .flags                 = {},
    .surface               = surface.vk(),
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

  Log(core::Trace, "Creating swapchain...");
  sc_data_ = std::make_shared<SwapchainImpl>(device, swapchain_ci);

  // Clear data from old swapchain
  image_views_.clear();
  images_.clear();

  // Get new swapchain images
  uint32_t image_count;
  if (const VkResult result{ vkGetSwapchainImagesKHR(
        device.logical(), sc_data_->swapchain_, &image_count, nullptr) };
      result != VK_SUCCESS)
    Throw("Failed to get swapchain images! ({})", result);

  images_.resize(image_count);

  if (const VkResult result{ vkGetSwapchainImagesKHR(
        device.logical(), sc_data_->swapchain_, &image_count, images_.data()) };
      result != VK_SUCCESS)
    Throw("Failed to get swapchain images! ({})", result);

  //----------------------------------------------------------------------------
  // Create image views
  //----------------------------------------------------------------------------
  image_views_.reserve(image_count);
  ImageViewCreateInfo image_view_ci{};
  image_view_ci.format       = surface_format_.format;
  image_view_ci.aspect_flags = VK_IMAGE_ASPECT_COLOR_BIT;
  image_view_ci.mip_levels   = 1;

  for (const VkImage image : images_) {
    image_view_ci.image = image;
    image_views_.emplace_back(device, image_view_ci);
  }
}

const VkSemaphore* Swapchain::imageAvailableSemaphore(uint32_t index) const
{
  return image_available_sem_[index].vkp();
}

const VkSemaphore* Swapchain::renderFinishedSemaphore(uint32_t index) const
{
  return render_finished_sem_[index].vkp();
}

uint32_t Swapchain::acquireNextImage(uint32_t frame_index,
                                     bool&    invalidate_swapchain) const
{
  uint32_t       image_index{ 0 };
  const VkResult result{ vkAcquireNextImageKHR(
    sc_data_->device_.logical(),
    sc_data_->swapchain_,
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
  const VkResult result{ vkQueuePresentKHR(sc_data_->device_.presentQueue(),
                                           &present_info) };
  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
    invalidate_swapchain = true;
  }
  else if (result != VK_SUCCESS) {
    Throw("Failed to present queue! ({})", result);
  }
}

VkSwapchainKHR  Swapchain::vk() const { return sc_data_->swapchain_; }
VkSwapchainKHR* Swapchain::vkp() const { return &sc_data_->swapchain_; }
} // namespace eldr::vk::wr
