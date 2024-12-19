#pragma once
#include <eldr/core/fwd.hpp>
#include <eldr/core/math.hpp>
#include <eldr/vulkan/common.hpp>
#include <eldr/vulkan/wrappers/imageview.hpp>

#include <optional>

namespace eldr::vk::wr {

class Image {
  ELDR_IMPORT_CORE_TYPES();

public:
  // Info required for complete Image creation
  struct ImageCreateInfo {
    std::string           name;
    VkExtent2D            extent;
    VkFormat              format;
    VkImageTiling         tiling;
    VkImageUsageFlags     usage_flags;
    VkImageAspectFlags    aspect_flags;
    VkSampleCountFlagBits sample_count{ VK_SAMPLE_COUNT_1_BIT };
    uint32_t              mip_levels{ 1 };
    VmaMemoryUsage        memory_usage{ VMA_MEMORY_USAGE_GPU_ONLY };
  };

  Image() = default;
  Image(const Device&, const ImageCreateInfo&);
  virtual ~Image() = default;

  [[nodiscard]] VkImage            get() const;
  [[nodiscard]] const std::string& name() const;
  [[nodiscard]] VkExtent2D         size() const { return size_; }
  [[nodiscard]] VkFormat           format() const { return format_; }
  [[nodiscard]] uint32_t           mipLevels() const { return mip_levels_; }
  [[nodiscard]] const ImageView&   imageView() const { return image_view_; }

protected:
  std::string name_;

  VkExtent2D size_;
  // uint32_t   channels_{ 1 };
  VkFormat format_;
  uint32_t mip_levels_{ 1 };

  class ImageImpl;
  std::shared_ptr<ImageImpl> i_data_;
  ImageView                  image_view_;
};

} // namespace eldr::vk::wr
