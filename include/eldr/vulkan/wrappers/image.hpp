#pragma once

#include <eldr/core/fwd.hpp>
#include <eldr/core/math.hpp>
#include <eldr/vulkan/common.hpp>
#include <eldr/vulkan/fwd.hpp>
#include <eldr/vulkan/rendergraph.hpp>

namespace eldr::vk::wr {
// Minimum info required for image creation
struct ImageInfo {
  VkExtent2D            extent;
  VkFormat              format;
  VkImageTiling         tiling;
  VkImageUsageFlags     usage_flags;
  VkImageAspectFlags    aspect_flags;
  VkSampleCountFlagBits num_samples;
  uint32_t              mip_levels;
};

class Image final : public vk::PhysicalResource {
  ELDR_IMPORT_CORE_TYPES();

public:
  Image(Image&) = delete;
  Image(const Device&, const ImageInfo&, VmaAllocationCreateInfo&);
  Image(const Device&, const Bitmap&, VmaAllocationCreateInfo&);
  Image(const Image&) = delete;
  Image(Image&&)      = delete;
  ~Image();

  void transitionLayout(CommandPool&, VkImageLayout old_layout,
                        VkImageLayout new_layout, uint32_t mip_levels);

  void copyFromBuffer(const Buffer&, CommandPool&);

  [[nodiscard]] VkImage get() const { return image_; }
  [[nodiscard]] VkImageView view() const { return image_view_; }
  [[nodiscard]] VkFormat   format() const { return format_; }
  [[nodiscard]] VkExtent2D size() const { return size_; }
  [[nodiscard]] uint32_t   mipLevels() const { return mip_levels_; }

protected:
  VkImage     image_{ VK_NULL_HANDLE };
  VkImageView image_view_{ VK_NULL_HANDLE };
  uint32_t    mip_levels_{ 1 };
  VkFormat    format_;
  VkExtent2D  size_;
};

//class ImageView {
//  ELDR_IMPORT_CORE_TYPES();
//
//public:
//  ImageView() = delete;
//  ImageView(const Device&, const Image&, VkImageAspectFlags);
//  ImageView(const ImageView&) = delete;
//  ImageView(ImageView&&)      = delete;
//  ~ImageView();
//
//  VkImageView get() const { return image_view_; }
//
//protected:
//  const Device& device_;
//
//  VkImageView image_view_{ VK_NULL_HANDLE };
//};
} // namespace eldr::vk::wr
