#include <eldr/core/logger.hpp>
#include <eldr/vulkan/helpers.hpp>
#include <eldr/vulkan/wrappers/commandbuffer.hpp>
#include <eldr/vulkan/wrappers/device.hpp>
#include <eldr/vulkan/wrappers/image.hpp>

namespace eldr::vk::wr {

Image::Image(Device& device, const ImageInfo& image_info)
  : device_(device), format_(image_info.format), size_(image_info.extent)
{
  // ---------------------------------------------------------------------------
  // Create image
  // ---------------------------------------------------------------------------
  VkFormatProperties format_properties;
  vkGetPhysicalDeviceFormatProperties(device_.physical(), image_info.format,
                                      &format_properties);

  auto image_ci          = makeInfo<VkImageCreateInfo>();
  image_ci.imageType     = VK_IMAGE_TYPE_2D;
  image_ci.extent.width  = size_.width;
  image_ci.extent.height = size_.height;
  image_ci.extent.depth  = 1;
  image_ci.mipLevels     = image_info.mip_levels;
  image_ci.arrayLayers   = 1;
  image_ci.format        = image_info.format;
  image_ci.tiling        = image_info.tiling;
  image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  image_ci.usage         = image_info.usage_flags;
  image_ci.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;
  image_ci.samples       = image_info.num_samples;
  image_ci.flags         = 0;

  CheckVkResult(vkCreateImage(device_.logical(), &image_ci, nullptr, &image_));

  // ---------------------------------------------------------------------------
  // Allocate memory
  // ---------------------------------------------------------------------------
  VkMemoryRequirements mem_requirements;
  vkGetImageMemoryRequirements(device_.logical(), image_, &mem_requirements);

  auto alloc_info            = makeInfo<VkMemoryAllocateInfo>();
  alloc_info.allocationSize  = mem_requirements.size;
  alloc_info.memoryTypeIndex = device_.findMemoryType(
    mem_requirements.memoryTypeBits, image_info.memory_flags);

  CheckVkResult(
    vkAllocateMemory(device_.logical(), &alloc_info, nullptr, &image_memory_));
  vkBindImageMemory(device_.logical(), image_, image_memory_, 0);
}

ImageView::ImageView(const Device& device, const Image& image,
                     const ImageInfo& image_info)
  : device_(device)
{
  auto image_view_ci     = makeInfo<VkImageViewCreateInfo>();
  image_view_ci.image    = image.get();
  image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
  image_view_ci.format   = image.format();
  // Standard color properties
  image_view_ci.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
  image_view_ci.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
  image_view_ci.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
  image_view_ci.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

  image_view_ci.subresourceRange.aspectMask     = image_info.aspect_flags;
  image_view_ci.subresourceRange.baseMipLevel   = 0;
  image_view_ci.subresourceRange.levelCount     = image_info.mip_levels;
  image_view_ci.subresourceRange.baseArrayLayer = 0;
  image_view_ci.subresourceRange.layerCount     = 1;

  CheckVkResult(vkCreateImageView(device_.logical(), &image_view_ci, nullptr,
                                  &image_view_));
}

Image::~Image()
{
  if (image_ != VK_NULL_HANDLE)
    vkDestroyImage(device_.logical(), image_, nullptr);
  if (image_memory_ != VK_NULL_HANDLE)
    vkFreeMemory(device_.logical(), image_memory_, nullptr);
}

ImageView::~ImageView()
{
  if (image_view_ != VK_NULL_HANDLE)
    vkDestroyImageView(device_.logical(), image_view_, nullptr);
}

void Image::transitionLayout(CommandPool&  command_pool,
                             VkImageLayout old_layout, VkImageLayout new_layout,
                             uint32_t mip_levels)
{
  CommandBuffer cb(device_, command_pool);
  cb.beginSingleCommand();

  VkImageMemoryBarrier barrier{};
  barrier.sType                       = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.oldLayout                   = old_layout;
  barrier.newLayout                   = new_layout;
  barrier.srcQueueFamilyIndex         = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex         = VK_QUEUE_FAMILY_IGNORED;
  barrier.image                       = image_;
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseMipLevel   = 0;
  barrier.subresourceRange.levelCount     = mip_levels;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount     = 1;

  VkPipelineStageFlags source_stage;
  VkPipelineStageFlags destination_stage;

  if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED &&
      new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    source_stage          = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destination_stage     = VK_PIPELINE_STAGE_TRANSFER_BIT;
  }
  else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
           new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    source_stage          = VK_PIPELINE_STAGE_TRANSFER_BIT;
    destination_stage     = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  }
  else {
    ThrowSpecific(std::invalid_argument, "Unsupported layout transition!");
  }

  vkCmdPipelineBarrier(cb.get(), source_stage, destination_stage, 0, 0, nullptr,
                       0, nullptr, 1, &barrier);

  cb.end();
  cb.submit();
}

void Image::copyFromBuffer(const Buffer& buffer, CommandPool& command_pool)
{
  CommandBuffer cb(device_, &command_pool);
  cb.beginSingleCommand();

  VkBufferImageCopy region{};
  region.bufferOffset      = 0;
  region.bufferRowLength   = 0;
  region.bufferImageHeight = 0;

  region.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
  region.imageSubresource.mipLevel       = 0;
  region.imageSubresource.baseArrayLayer = 0;
  region.imageSubresource.layerCount     = 1;

  region.imageOffset = { 0, 0, 0 };
  region.imageExtent = { size_.x, size_.y, 1 };

  vkCmdCopyBufferToImage(cb.get(), buffer.get(), image_,
                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

  cb.submit();
}
} // namespace eldr::vk::wr
