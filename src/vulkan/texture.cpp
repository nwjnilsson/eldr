#include <eldr/core/fstream.hpp>
#include <eldr/core/logger.hpp>
#include <eldr/vulkan/buffer.hpp>
#include <eldr/vulkan/helpers.hpp>
#include <eldr/vulkan/texture.hpp>
#include <vulkan/vulkan_core.h>

namespace eldr {
namespace vk {

Texture::Texture(const Device* device, CommandPool& command_pool)
{
  const char* env_p = std::getenv("ELDR_DIR");
  if (env_p == nullptr) {
    Throw("Environment not set up correctly");
  }
  std::filesystem::path filepath =
    std::string(env_p) + "/resources/texture.jpg";

  auto fstream = std::make_unique<FileStream>(filepath, FileStream::ERead);

  // use bitmap class somehow
  auto bitmap = std::make_unique<Bitmap>(filepath);

  VkFormat image_format;
  if (bitmap->pixelFormat() == Bitmap::PixelFormat::RGBA)
    image_format = VK_FORMAT_R8G8B8A8_SRGB;
  else
    Throw("Unsupported image format!");

  VkDeviceSize image_size =
    bitmap->width() * bitmap->height() * bitmap->channelCount();

  BufferInfo buffer_info{ .size       = image_size,
                          .usage      = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                          .properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT };

  Buffer staging_buffer(device, buffer_info);

  void* data;
  vkMapMemory(device->logical(), staging_buffer.memory(), 0, image_size, 0,
              &data);
  memcpy(data, bitmap->data(), static_cast<size_t>(image_size));
  vkUnmapMemory(device->logical(), staging_buffer.memory());

  // TODO: write move semantics instead of using unique ptr
  ImageInfo image_info{ .bitmap = bitmap.get(),
                        .format = image_format,
                        .tiling = VK_IMAGE_TILING_OPTIMAL,
                        .usage  = VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                 VK_IMAGE_USAGE_SAMPLED_BIT,
                        .properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT };
  image_ = Image(device, image_info);

  image_.transitionLayout(command_pool, VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

  image_.copyFromBuffer(staging_buffer, command_pool);

  image_.transitionLayout(command_pool, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                          VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

  // TODO: this was createTextureImageView(), should
  // maybe not be placed here
  image_view_ = ImageView(device, image_.get(), image_format);
}

// Texture& Texture::operator=(Texture&& other) {
//   if (this != &other) {
//     image_ = other.image_;
//     image_view_ = other.image_view_;
//   }
//   return *this;
// }

} // namespace vk
} // namespace eldr
