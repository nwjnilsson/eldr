#include <eldr/core/fstream.hpp>
#include <eldr/core/logger.hpp>
#include <eldr/vulkan/buffer.hpp>
#include <eldr/vulkan/helpers.hpp>
#include <eldr/vulkan/texture.hpp>
#include <eldr/render/scene.hpp>

namespace eldr {
namespace vk {

Texture::Texture(const Device* device, CommandPool& command_pool)
{
  const char* env_p = std::getenv("ELDR_DIR");
  if (env_p == nullptr) {
    Throw("Environment not set up correctly");
  }
  // FIXME: set this some other way
  const std::string texture_path = "/textures/viking_room.png";
  std::filesystem::path filepath =
    std::string(env_p) + texture_path;

  // use bitmap class somehow
  auto bitmap = std::make_unique<Bitmap>(filepath);
  if (bitmap->pixelFormat() != Bitmap::PixelFormat::RGBA)
    bitmap->rgbToRgba();

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
  image_ = Image(device, *bitmap);

  image_.transitionLayout(command_pool, VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

  image_.copyFromBuffer(staging_buffer, command_pool);

  image_.transitionLayout(command_pool, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                          VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

  // TODO: this was createTextureImageView(), should
  // maybe not be placed here
  image_view_ = ImageView(device, image_.get(), image_.format());
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
