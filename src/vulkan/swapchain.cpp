#include <eldr/core/logger.hpp>
#include <eldr/vulkan/helpers.hpp>
#include <eldr/vulkan/swapchain.hpp>

namespace eldr {
namespace vk {
// fwd -------------------------------------------------------------------------
static VkSurfaceFormatKHR
selectSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>&);
static VkPresentModeKHR
                  selectSwapPresentMode(const std::vector<VkPresentModeKHR>&);
static VkExtent2D selectSwapExtent(GLFWwindow*,
                                   const VkSurfaceCapabilitiesKHR&);
// -----------------------------------------------------------------------------

Swapchain::Swapchain(const Device* device, Surface& surface, GLFWwindow* window)
  : device_(device)
{
  SwapChainSupportDetails swapchain_support =
    swapChainSupportDetails(device_->physical(), surface.get());
  surface.setFormat(selectSwapSurfaceFormat(swapchain_support.formats));
  surface.setPresentMode(
    selectSwapPresentMode(swapchain_support.present_modes));
  VkExtent2D extent = selectSwapExtent(window, swapchain_support.capabilities);

  // TODO: keep this image count?
  min_image_count_ = swapchain_support.capabilities.minImageCount + 1;
  // If there is an upper limit, make sure we don't exceed it
  if (swapchain_support.capabilities.maxImageCount > 0) {
    min_image_count_ =
      std::min(min_image_count_, swapchain_support.capabilities.maxImageCount);
  }
  VkSwapchainCreateInfoKHR swap_ci{};
  swap_ci.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  swap_ci.surface          = surface.get();
  swap_ci.minImageCount    = min_image_count_;
  swap_ci.imageFormat      = surface.format().format;
  swap_ci.imageColorSpace  = surface.format().colorSpace;
  swap_ci.imageExtent      = extent;
  swap_ci.imageArrayLayers = 1;
  swap_ci.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  swap_ci.preTransform     = swapchain_support.capabilities.currentTransform;
  swap_ci.compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  swap_ci.presentMode      = surface.presentMode();
  swap_ci.clipped      = VK_TRUE; // don't care about color of obscured pixels
  swap_ci.oldSwapchain = VK_NULL_HANDLE;

  QueueFamilyIndices indices =
    findQueueFamilies(device_->physical(), surface.get());
  uint32_t queueFamilyIndices[] = { indices.graphics_family.value(),
                                    indices.present_family.value() };

  if (indices.graphics_family != indices.present_family) {
    swap_ci.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
    swap_ci.queueFamilyIndexCount = 2;
    swap_ci.pQueueFamilyIndices   = queueFamilyIndices;
  }
  else {
    swap_ci.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
    swap_ci.queueFamilyIndexCount = 0;       // Optional
    swap_ci.pQueueFamilyIndices   = nullptr; // Optional
  }

  if (vkCreateSwapchainKHR(device_->logical(), &swap_ci, nullptr,
                           &swapchain_) != VK_SUCCESS) {
    ThrowVk("Failed to create swapchain");
  }
  vkGetSwapchainImagesKHR(device_->logical(), swapchain_, &image_count_,
                          nullptr);
  images_.resize(image_count_);
  vkGetSwapchainImagesKHR(device_->logical(), swapchain_, &image_count_,
                          images_.data());
  extent_       = extent;
  image_format_ = surface.format().format;

  // Create image views
  for (size_t i = 0; i < images_.size(); i++) {
    image_views_.emplace_back(ImageView(device_, images_[i], image_format_));
  }
}

Swapchain::~Swapchain() { 
  if (swapchain_ != VK_NULL_HANDLE)
    vkDestroySwapchainKHR(device_->logical(), swapchain_, nullptr);

  for (size_t i = 0; i < framebuffers_.size(); ++i)
    if (framebuffers_[i] != VK_NULL_HANDLE)
      vkDestroyFramebuffer(device_->logical(), framebuffers_[i], nullptr);
}

void Swapchain::createFramebuffers(RenderPass& render_pass)
{
  framebuffers_.resize(image_views_.size());
  for (size_t i = 0; i < image_views_.size(); ++i) {
    VkImageView             attachments[] = { image_views_[i].get() };
    VkFramebufferCreateInfo framebuffer_ci{};
    framebuffer_ci.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebuffer_ci.renderPass      = render_pass.get();
    framebuffer_ci.attachmentCount = 1;
    framebuffer_ci.pAttachments    = attachments;
    framebuffer_ci.width           = extent_.width;
    framebuffer_ci.height          = extent_.height;
    framebuffer_ci.layers          = 1;

    if (vkCreateFramebuffer(device_->logical(), &framebuffer_ci, nullptr,
                            &framebuffers_[i]) != VK_SUCCESS)
      ThrowVk("Failed to create framebuffer!");
  }
}

/** TODO: implement if needed
 *
void Swapchain::recreate() {
  vkDeviceWaitIdle(p_device_->logical());
  clean();
  ...
}
*/

// -----------------------------------------------------------------------------
static VkSurfaceFormatKHR selectSwapSurfaceFormat(
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

static VkPresentModeKHR selectSwapPresentMode(
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

static VkExtent2D selectSwapExtent(GLFWwindow*                     window,
                                   const VkSurfaceCapabilitiesKHR& capabilities)
{
  // Match the resolution of the current window, unless the value is the max
  // of uint32_t
  if (capabilities.currentExtent.width !=
      std::numeric_limits<uint32_t>::max()) {
    return capabilities.currentExtent;
  }
  else {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    VkExtent2D extent = { static_cast<uint32_t>(width),
                          static_cast<uint32_t>(height) };
    extent.width = std::clamp(extent.width, capabilities.minImageExtent.width,
                              capabilities.maxImageExtent.width);
    extent.height =
      std::clamp(extent.height, capabilities.minImageExtent.height,
                 capabilities.maxImageExtent.height);
    return extent;
  }
}
} // namespace vk
} // namespace eldr
