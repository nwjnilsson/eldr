#include <eldr/vulkan/wrappers/buffer.hpp>
#include <eldr/vulkan/wrappers/commandbuffer.hpp>
#include <eldr/vulkan/wrappers/device.hpp>
#include <eldr/vulkan/wrappers/image.hpp>

namespace eldr::vk::wr {

GpuImage::GpuImage(const Device& device, const ImageInfo& image_info,
                   const VmaAllocationCreateInfo& alloc_ci,
                   const std::string&             name)
  : GpuResource(device, name), format_(image_info.format),
    size_(image_info.extent)
{
  // ---------------------------------------------------------------------------
  // Create image view
  // ---------------------------------------------------------------------------
  VkImageCreateInfo image_ci{
    .sType                 = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
    .pNext                 = nullptr,
    .flags                 = 0,
    .imageType             = VK_IMAGE_TYPE_2D,
    .format                = format_,
    .extent                = { size_.width, size_.height, 1 },
    .mipLevels             = image_info.mip_levels,
    .arrayLayers           = 1,
    .samples               = image_info.sample_count,
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
    ThrowVk(result, "vmaCreateImage(): ");

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
      .levelCount     = image_info.mip_levels,
      .baseArrayLayer = 0,
      .layerCount     = 1,
    },
  };
  device_.createImageView(image_view_ci, &image_view_, name);
}

GpuImage::~GpuImage()
{
  vmaDestroyImage(device_.allocator(), image_, allocation_);
  vkDestroyImageView(device_.logical(), image_view_, nullptr);
}
} // namespace eldr::vk::wr
