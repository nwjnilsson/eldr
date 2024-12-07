#pragma once

#include <eldr/core/fwd.hpp>
#include <eldr/core/math.hpp>
#include <eldr/vulkan/common.hpp>
#include <eldr/vulkan/fwd.hpp>
#include <eldr/vulkan/wrappers/imageview.hpp>

namespace eldr::vk::wr {
// Minimum info required for image creation
struct ImageInfo {
  VkExtent2D            extent;
  VkFormat              format;
  VkImageTiling         tiling;
  VkImageUsageFlags     usage_flags;
  VkImageAspectFlags    aspect_flags;
  VkSampleCountFlagBits sample_count;
  uint32_t              mip_levels;
};

class Image final {
  ELDR_IMPORT_CORE_TYPES();

public:
  Image(const Device&, std::string_view name, const ImageInfo&,
        VmaMemoryUsage memory_usage = VMA_MEMORY_USAGE_GPU_ONLY);
  Image(const Device&, std::string_view name, const Bitmap&,
        VmaMemoryUsage memory_usage = VMA_MEMORY_USAGE_GPU_ONLY);

  [[nodiscard]] VkImage     get() const { return image_; }
  [[nodiscard]] VkImageView view() const { return image_view_; }
  [[nodiscard]] VkFormat    format() const { return format_; }
  [[nodiscard]] VkExtent2D  size() const { return size_; }
  [[nodiscard]] uint32_t    mipLevels() const { return mip_levels_; }

private:
  std::string name_;
  class ImageImpl;
  std::shared_ptr<ImageImpl> i_data_;

  VkImage    image_{ VK_NULL_HANDLE };
  uint32_t   mip_levels_{ 1 };
  ImageView  image_view_;
  VkFormat   format_;
  VkExtent2D size_;
};

} // namespace eldr::vk::wr
