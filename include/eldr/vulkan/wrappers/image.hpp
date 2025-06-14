#pragma once
#include <eldr/core/fwd.hpp>
#include <eldr/vulkan/wrappers/imageview.hpp>

namespace eldr::vk::wr {
/// Info required to create image, image view, and allocating memory
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
  VkImageLayout         final_layout{ VK_IMAGE_LAYOUT_UNDEFINED };
};

class Image {
  ELDR_IMPORT_CORE_TYPES();

public:
  Image();
  Image(const Device&, const ImageCreateInfo&);
  Image(const Device&, const Bitmap&);
  Image(const Device&, const Bitmap&, uint32_t mip_levels);
  Image(Image&&) noexcept;
  ~Image();

  Image& operator=(Image&&);

  [[nodiscard]] static Image createSwapchainImage(
    const Device&, VkImage, std::string_view name, VkExtent2D, VkFormat);

  [[nodiscard]] VkImage            vk() const;
  [[nodiscard]] const std::string& name() const;
  [[nodiscard]] VkExtent2D         size() const { return size_; }
  [[nodiscard]] VkFormat           format() const { return format_; }
  [[nodiscard]] uint32_t           mipLevels() const { return mip_levels_; }

  /// @brief Returns the layout of this image
  [[nodiscard]] VkImageLayout layout() const { return layout_; }

  [[nodiscard]] const ImageView& view() const { return image_view_; }

  void setLayout(VkImageLayout new_layout) { layout_ = new_layout; }

protected:
  std::string name_;

  VkExtent2D size_;
  VkFormat   format_;
  uint32_t   mip_levels_{ 1 };
  // One thing to keep in mind is that render passes will "implicitly"
  // transition the layouts of images according to how the attachments etc are
  // set up. But I'm using dynamic rendering now so I shouldn't be face any
  // surprises with unexpected layout changes.
  VkImageLayout layout_{ VK_IMAGE_LAYOUT_UNDEFINED };
  // uint32_t   channels_{ 1 };

  ImageView image_view_;
  class ImageImpl;
  std::unique_ptr<ImageImpl> d_;
};

} // namespace eldr::vk::wr
