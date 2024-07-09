#include <eldr/vulkan/command.hpp>
#include <eldr/vulkan/helpers.hpp>
#include <eldr/vulkan/image.hpp>

namespace eldr {
namespace vk {
Image::Image() : image_(VK_NULL_HANDLE), image_memory_(VK_NULL_HANDLE) {}

Image::Image(const Device* device, const ImageInfo& image_info)
  : device_(device), format_(image_info.format),
    width_(image_info.extent.width), height_(image_info.extent.height)
{
  VkFormatProperties format_properties;
  vkGetPhysicalDeviceFormatProperties(device_->physical(), image_info.format,
                                      &format_properties);

  VkImageCreateInfo image_ci{};
  image_ci.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  image_ci.imageType     = VK_IMAGE_TYPE_2D;
  image_ci.extent.width  = width_;
  image_ci.extent.height = height_;
  image_ci.extent.depth  = 1;
  image_ci.mipLevels     = 1;
  image_ci.arrayLayers   = 1;
  image_ci.format        = image_info.format;
  image_ci.tiling        = image_info.tiling;
  image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  image_ci.usage         = image_info.usage;
  image_ci.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;
  image_ci.samples       = VK_SAMPLE_COUNT_1_BIT;
  image_ci.flags         = 0;

  if (vkCreateImage(device_->logical(), &image_ci, nullptr, &image_) !=
      VK_SUCCESS)
    ThrowVk("Failed to create image!");

  VkMemoryRequirements mem_requirements;
  vkGetImageMemoryRequirements(device_->logical(), image_, &mem_requirements);

  VkMemoryAllocateInfo alloc_info{};
  alloc_info.sType          = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  alloc_info.allocationSize = mem_requirements.size;
  alloc_info.memoryTypeIndex =
    findMemoryType(device_->physical(), mem_requirements.memoryTypeBits,
                   image_info.properties);

  if (vkAllocateMemory(device_->logical(), &alloc_info, nullptr,
                       &image_memory_) != VK_SUCCESS)
    ThrowVk("Failed to allocate memory!");

  vkBindImageMemory(device_->logical(), image_, image_memory_, 0);
}

ImageInfo getImageInfoFromBitmap(const Bitmap& bitmap)
{
  VkFormat image_format;
  if (bitmap.pixelFormat() == Bitmap::PixelFormat::RGBA)
    image_format = VK_FORMAT_R8G8B8A8_SRGB;
  else
    Throw("Unsupported image format!");

  ImageInfo image_info{ .extent = { bitmap.width(), bitmap.height() },
                        .format = image_format,
                        .tiling = VK_IMAGE_TILING_OPTIMAL,
                        .usage  = VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                 VK_IMAGE_USAGE_SAMPLED_BIT,
                        .properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT };
  return image_info;
}

Image::Image(const Device* device, const Bitmap& bitmap)
  : Image(device, getImageInfoFromBitmap(bitmap))
{
}

Image::~Image()
{
  if (image_ != VK_NULL_HANDLE)
    vkDestroyImage(device_->logical(), image_, nullptr);
  if (image_memory_ != VK_NULL_HANDLE)
    vkFreeMemory(device_->logical(), image_memory_, nullptr);
}

Image& Image::operator=(Image&& other)
{
  if (this != &other) {
    if (image_ != VK_NULL_HANDLE)
      vkDestroyImage(device_->logical(), image_, nullptr);
    if (image_memory_ != VK_NULL_HANDLE)
      vkFreeMemory(device_->logical(), image_memory_, nullptr);
    device_       = other.device_;
    image_        = other.image_;
    image_memory_ = other.image_memory_;
    format_       = other.format_;
    width_        = other.width_;
    height_       = other.height_;

    other.device_       = nullptr;
    other.image_        = VK_NULL_HANDLE;
    other.image_memory_ = VK_NULL_HANDLE;
    other.width_        = 0;
    other.height_       = 0;
  }
  return *this;
}

// TODO: figure out command buffers
void Image::transitionLayout(CommandPool&  command_pool,
                             VkImageLayout old_layout, VkImageLayout new_layout)
{
  SingleTimeCommand command(device_, &command_pool);

  VkImageMemoryBarrier barrier{};
  barrier.sType                       = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.oldLayout                   = old_layout;
  barrier.newLayout                   = new_layout;
  barrier.srcQueueFamilyIndex         = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex         = VK_QUEUE_FAMILY_IGNORED;
  barrier.image                       = image_;
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseMipLevel   = 0;
  barrier.subresourceRange.levelCount     = 1;
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

  vkCmdPipelineBarrier(command.buffer(), source_stage, destination_stage, 0, 0,
                       nullptr, 0, nullptr, 1, &barrier);

  command.submit();
}

void Image::copyFromBuffer(const Buffer& buffer, CommandPool& command_pool)
{
  SingleTimeCommand command(device_, &command_pool);

  VkBufferImageCopy region{};
  region.bufferOffset      = 0;
  region.bufferRowLength   = 0;
  region.bufferImageHeight = 0;

  region.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
  region.imageSubresource.mipLevel       = 0;
  region.imageSubresource.baseArrayLayer = 0;
  region.imageSubresource.layerCount     = 1;

  region.imageOffset = { 0, 0, 0 };
  region.imageExtent = { width_, height_, 1 };

  vkCmdCopyBufferToImage(command.buffer(), buffer.get(), image_,
                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

  command.submit();
}
} // namespace vk
} // namespace eldr
