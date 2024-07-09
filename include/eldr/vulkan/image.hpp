#pragma once

#include <eldr/core/bitmap.hpp>
#include <eldr/vulkan/buffer.hpp>
#include <eldr/vulkan/commandpool.hpp>
#include <eldr/vulkan/common.hpp>
#include <eldr/vulkan/device.hpp>

namespace eldr {
namespace vk {
struct ImageInfo {
  VkExtent2D            extent;
  VkFormat              format;
  VkImageTiling         tiling;
  VkImageUsageFlags     usage;
  VkMemoryPropertyFlags properties;
};

class Image {
public:
  Image();
  Image(const Device*, const ImageInfo&);
  Image(const Device*, const Bitmap&);
  ~Image();

  Image& operator=(Image&&);

  void transitionLayout(CommandPool&, VkImageLayout old_layout,
                        VkImageLayout new_layout);

  void copyFromBuffer(const Buffer&, CommandPool&);

  const VkImage&  get() const { return image_; }
  const VkFormat& format() const { return format_; }

protected:
  const Device* device_;

  VkImage        image_;
  VkFormat       format_;
  VkDeviceMemory image_memory_;
  uint32_t       width_;
  uint32_t       height_;
};
} // namespace vk
} // namespace eldr
