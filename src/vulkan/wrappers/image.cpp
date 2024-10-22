#include <eldr/core/logger.hpp>
#include <eldr/vulkan/exception.hpp>
#include <eldr/vulkan/helpers.hpp>
#include <eldr/vulkan/wrappers/buffer.hpp>
#include <eldr/vulkan/wrappers/commandbuffer.hpp>
#include <eldr/vulkan/wrappers/device.hpp>
#include <eldr/vulkan/wrappers/image.hpp>

namespace eldr::vk::wr {

GpuImage::GpuImage(const Device& device, const ImageInfo& image_info,
             const VmaAllocationCreateInfo& alloc_ci, const std::string& name)
  : GpuResource(device, name), mip_levels_(image_info.mip_levels),
    format_(image_info.format), size_(image_info.extent)
{
  // --------------------------------------------------------------------------- 
  // Create image view
  // --------------------------------------------------------------------------- 
  VkImageCreateInfo image_ci{
    .sType                 = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
    .pNext                 = nullptr,
    .flags                 = 0,
    .imageType             = VK_IMAGE_TYPE_2D,
    .format                = image_info.format,
    .extent                = { size_.width, size_.height, 1 },
    .mipLevels             = image_info.mip_levels,
    .arrayLayers           = 1,
    .samples               = image_info.num_samples,
    .tiling                = image_info.tiling,
    .usage                 = image_info.usage_flags,
    .sharingMode           = VK_SHARING_MODE_EXCLUSIVE,
    .queueFamilyIndexCount = {},
    .pQueueFamilyIndices   = nullptr,
    .initialLayout         = VK_IMAGE_LAYOUT_UNDEFINED,
  };

  VmaAllocationInfo alloc_info;
  if (const VkResult result =
        vmaCreateImage(device_.allocator(), &image_ci, &alloc_ci, &image_,
                       &allocation_, &alloc_info);
      result != VK_SUCCESS)
    ThrowVk(result, "vmaCreateImage(): failed to create image!");

  // --------------------------------------------------------------------------- 
  // Create image view
  // --------------------------------------------------------------------------- 
  const VkImageViewCreateInfo image_view_ci{
    .sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
    .pNext = nullptr,
    .flags = {},
    .image    = image_,
    .viewType = VK_IMAGE_VIEW_TYPE_2D,
    .format   = format_,
    // Standard color properties
    .components = { VK_COMPONENT_SWIZZLE_IDENTITY,
                    VK_COMPONENT_SWIZZLE_IDENTITY,
                    VK_COMPONENT_SWIZZLE_IDENTITY,
                    VK_COMPONENT_SWIZZLE_IDENTITY },
    .subresourceRange = { 
      .aspectMask     = image_info.aspect_flags,
      .baseMipLevel   = 0,
      .levelCount     = mip_levels_,
      .baseArrayLayer = 0,
      .layerCount     = 1,
    },
  };
  if (const VkResult result = vkCreateImageView(
        device_.logical(), &image_view_ci, nullptr, &image_view_);
      result != VK_SUCCESS)
    ThrowVk(result, "vkCreateImageView(): failed to create image view!");
}

//ImageView::ImageView(const Device& device, const Image& image,
//                     VkImageAspectFlags aspect_flags)
//  : device_(device)
//{
//  const VkImageViewCreateInfo image_view_ci{
//    .sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
//    .pNext = nullptr,
//    .flags = {},
//    .image    = image.get(),
//    .viewType = VK_IMAGE_VIEW_TYPE_2D,
//    .format   = image.format(),
//    // Standard color properties
//    .components = { VK_COMPONENT_SWIZZLE_IDENTITY,
//                    VK_COMPONENT_SWIZZLE_IDENTITY,
//                    VK_COMPONENT_SWIZZLE_IDENTITY,
//                    VK_COMPONENT_SWIZZLE_IDENTITY },
//    .subresourceRange = { 
//      .aspectMask     = aspect_flags,
//      .baseMipLevel   = 0,
//      .levelCount     = image.mipLevels(),
//      .baseArrayLayer = 0,
//      .layerCount     = 1,
//    },
//  };
//  if (const VkResult result = vkCreateImageView(
//        device_.logical(), &image_view_ci, nullptr, &image_view_);
//      result != VK_SUCCESS)
//    ThrowVk(result, "vkCreateImageView(): failed to create image view!");
//}

Image::~Image()
{
  vmaDestroyImage(device_.allocator(), image_, allocation_);
  vkDestroyImageView(device_.logical(), image_view_, nullptr);
}

//ImageView::~ImageView()
//{
//  if (image_view_ != VK_NULL_HANDLE)
//    vkDestroyImageView(device_.logical(), image_view_, nullptr);
//}

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
    ThrowVk({}, "Unsupported layout transition!");
  }

  vkCmdPipelineBarrier(cb.get(), source_stage, destination_stage, 0, 0, nullptr,
                       0, nullptr, 1, &barrier);

  cb.end();
  cb.submit();
}

void Image::copyFromBuffer(const Buffer& buffer, CommandPool& command_pool)
{
  CommandBuffer cb(device_, command_pool);
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
  region.imageExtent = { size_.width, size_.height, 1 };

  vkCmdCopyBufferToImage(cb.get(), buffer.get(), image_,
                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

  cb.submit();
}
} // namespace eldr::vk::wr
