#pragma once

#include <eldr/core/fwd.hpp>
#include <eldr/core/math.hpp>
#include <eldr/vulkan/common.hpp>
#include <eldr/vulkan/fwd.hpp>
#include <eldr/vulkan/wrappers/gpuresource.hpp>

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

class GpuImage final : public GpuResource {
  ELDR_IMPORT_CORE_TYPES();

public:
  GpuImage(GpuImage&) = delete;
  GpuImage(const Device&, const ImageInfo&, const VmaAllocationCreateInfo&,
           const std::string& name);
  GpuImage(const Device&, const core::Bitmap&, const VmaAllocationCreateInfo&,
           const std::string& name);
  GpuImage(const GpuImage&) = delete;
  GpuImage(GpuImage&&)      = delete;
  ~GpuImage();

  [[nodiscard]] VkImage     get() const { return image_; }
  [[nodiscard]] VkImageView view() const { return image_view_; }
  [[nodiscard]] VkFormat    format() const { return format_; }
  [[nodiscard]] VkExtent2D  size() const { return size_; }

protected:
  VkImage     image_{ VK_NULL_HANDLE };
  VkImageView image_view_{ VK_NULL_HANDLE };
  VkFormat    format_;
  VkExtent2D  size_;
};

// class ImageView {
//   ELDR_IMPORT_CORE_TYPES();
//
// public:
//   ImageView() = delete;
//   ImageView(const Device&, const GpuImage&, VkImageAspectFlags);
//   ImageView(const ImageView&) = delete;
//   ImageView(ImageView&&)      = delete;
//   ~ImageView();
//
//   VkImageView get() const { return image_view_; }
//
// protected:
//   const Device& device_;
//
//   VkImageView image_view_{ VK_NULL_HANDLE };
// };
} // namespace eldr::vk::wr
