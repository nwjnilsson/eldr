#pragma once

#include <eldr/core/fwd.hpp>
#include <eldr/core/math.hpp>
#include <eldr/vulkan/common.hpp>
#include <eldr/vulkan/fwd.hpp>

namespace eldr::vk::wr {
// Minimum info required for image creation
struct ImageInfo {
  VkExtent2D            extent;
  VkFormat              format;
  VkImageTiling         tiling;
  VkImageUsageFlags     usage_flags;
  VkMemoryPropertyFlags memory_flags;
  VkImageAspectFlagBits aspect_flags;
  VkSampleCountFlagBits num_samples;
  uint32_t              mip_levels;
};

class Image {
  ELDR_IMPORT_CORE_TYPES();

public:
  Image(Image&) = delete;
  Image(Device&, const ImageInfo&);
  Image(Device&, const Bitmap&);
  Image(Image&&);
  ~Image();

  void transitionLayout(CommandPool&, VkImageLayout old_layout,
                        VkImageLayout new_layout, uint32_t mip_levels);

  void copyFromBuffer(const Buffer&, CommandPool&);

  VkImage     get() const { return image_; }
  VkImageView view() const { return image_view_; }
  VkFormat    format() const { return format_; }
  VkExtent2D  size() const { return size_; }

protected:
  Device& device_;

  VkImage        image_{ VK_NULL_HANDLE };
  VkFormat       format_;
  VkDeviceMemory image_memory_{ VK_NULL_HANDLE };
  VkExtent2D     size_;
  VkImageView    image_view_{ VK_NULL_HANDLE };
};
} // namespace eldr::vk::wr
