#include <eldr/core/logger.hpp>
#include <eldr/vulkan/helpers.hpp>
#include <eldr/vulkan/wrappers/device.hpp>
#include <eldr/vulkan/wrappers/image.hpp>
#include <eldr/vulkan/wrappers/renderpass.hpp>
#include <eldr/vulkan/wrappers/semaphore.hpp>
#include <eldr/vulkan/wrappers/surface.hpp>
#include <eldr/vulkan/wrappers/swapchain.hpp>
#include <eldr/vulkan/wrappers/window.hpp>

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

} // namespace

Swapchain::Swapchain(Device& device, Surface& surface, VkExtent2D extent,
                     const SwapchainSupportDetails& support_details)
  : device_(device), surface_(surface), extent_(extent),
    msaa_samples_(device_.getMaxMSAASampleCount())
{

  render_pass_ = std::make_unique<RenderPass>(device_, surface_format_.format,
                                              msaa_samples_);

  createSwapchain(support_details);
  createImageViews();
  createFramebuffers();
}

Swapchain::~Swapchain() { cleanup(); }

void Swapchain::createSwapchain(const SwapchainSupportDetails& support_details)
{
  ImageInfo color_image_info{ extent_,
                              surface_format_.format,
                              VK_IMAGE_TILING_OPTIMAL,
                              VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT |
                                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                              VK_IMAGE_ASPECT_COLOR_BIT,
                              msaa_samples_,
                              1 };
  color_image_ = std::make_unique<Image>(device_, color_image_info);

  ImageInfo depth_image_info{ extent_,
                              device_.findDepthFormat(),
                              VK_IMAGE_TILING_OPTIMAL,
                              VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                              VK_IMAGE_ASPECT_DEPTH_BIT,
                              msaa_samples_,
                              1 };

  depth_image_ = std::make_unique<Image>(device_, depth_image_info);

  surface_format_ = selectSwapSurfaceFormat(support_details.formats);
  present_mode_   = selectSwapPresentMode(support_details.present_modes);

  min_image_count_ = support_details.capabilities.minImageCount + 1;
  // If there is an upper limit, make sure we don't exceed it
  if (support_details.capabilities.maxImageCount > 0) {
    min_image_count_ =
      std::min(min_image_count_, support_details.capabilities.maxImageCount);
  }
  VkSwapchainCreateInfoKHR swap_ci{};
  swap_ci.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  swap_ci.surface          = surface_.get();
  swap_ci.minImageCount    = min_image_count_;
  swap_ci.imageFormat      = surface_format_.format;
  swap_ci.imageColorSpace  = surface_format_.colorSpace;
  swap_ci.imageExtent      = extent_;
  swap_ci.imageArrayLayers = 1;
  swap_ci.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  swap_ci.preTransform     = support_details.capabilities.currentTransform;
  swap_ci.compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  swap_ci.presentMode      = present_mode_;
  swap_ci.clipped      = VK_TRUE; // don't care about color of obscured pixels
  swap_ci.oldSwapchain = VK_NULL_HANDLE;

  QueueFamilyIndices indices =
    findQueueFamilies(device_.physical(), surface_.get());
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

  if (vkCreateSwapchainKHR(device_.logical(), &swap_ci, nullptr, &swapchain_) !=
      VK_SUCCESS) {
    ThrowVk("Failed to create swapchain");
  }
  vkGetSwapchainImagesKHR(device_.logical(), swapchain_, &image_count_,
                          nullptr);
  images_.resize(image_count_);
  vkGetSwapchainImagesKHR(device_.logical(), swapchain_, &image_count_,
                          images_.data());
}

void Swapchain::createImageViews()
{
  for (size_t i = 0; i < images_.size(); i++) {
    VkImageView image_view = device_.createImageView(
      images_[i], surface_format_.format, VK_IMAGE_ASPECT_COLOR_BIT, 1);
    image_views_.push_back(image_view);
  }
}

void Swapchain::cleanup()
{
  if (swapchain_ != VK_NULL_HANDLE)
    vkDestroySwapchainKHR(device_.logical(), swapchain_, nullptr);

  while (!image_views_.empty()) {
    vkDestroyImageView(device_.logical(), image_views_.back(), nullptr);
    image_views_.pop_back();
  }

  for (size_t i = 0; i < framebuffers_.size(); ++i)
    if (framebuffers_[i] != VK_NULL_HANDLE)
      vkDestroyFramebuffer(device_.logical(), framebuffers_[i], nullptr);
}

void Swapchain::createFramebuffers()
{
  size_t n_images = images_.size();
  framebuffers_.resize(n_images);
  for (size_t i = 0; i < n_images; ++i) {
    std::array<VkImageView, 3> attachments = { color_image_->view(),
                                               depth_image_->view(),
                                               image_views_[i] };
    VkFramebufferCreateInfo    framebuffer_ci{};
    framebuffer_ci.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebuffer_ci.renderPass      = render_pass_->get();
    framebuffer_ci.attachmentCount = static_cast<uint32_t>(attachments.size());
    framebuffer_ci.pAttachments    = attachments.data();
    framebuffer_ci.width           = extent_.width;
    framebuffer_ci.height          = extent_.height;
    framebuffer_ci.layers          = 1;

    if (vkCreateFramebuffer(device_.logical(), &framebuffer_ci, nullptr,
                            &framebuffers_[i]) != VK_SUCCESS)
      ThrowVk("Failed to create framebuffer!");
  }
}

void Swapchain::recreate(Window& window)
{

  device_.waitIdle();
  cleanup();

  auto support_details =
    getSwapchainSupportDetails(device_.physical(), surface_.get());
  extent_ = window.selectSwapExtent(support_details.capabilities);

  createSwapchain(support_details);
  createImageViews();
  createFramebuffers();
}

VkResult Swapchain::acquireNextImage(uint32_t& index, Semaphore& semaphore)
{
  return vkAcquireNextImageKHR(device_.logical(), swapchain_, UINT64_MAX,
                               semaphore.get(), VK_NULL_HANDLE, &index);
}

} // namespace eldr::vk::wr
