#pragma once

#include <eldr/vulkan/common.hpp>

#include <cassert>
#include <cstdint>
#include <string>
#include <vector>

namespace eldr::vk::wr {

class DescriptorBuilder {
public:
  /// @brief Constructs the descriptor builder.
  /// @param device The const reference to a device RAII wrapper instance.
  explicit DescriptorBuilder(const Device& device);

  DescriptorBuilder(const DescriptorBuilder&) = delete;
  DescriptorBuilder(DescriptorBuilder&&)      = delete;
  ~DescriptorBuilder()                        = default;

  DescriptorBuilder& operator=(const DescriptorBuilder&) = delete;
  DescriptorBuilder& operator=(DescriptorBuilder&&)      = delete;

  /// @brief Adds a uniform buffer to the descriptor container.
  /// @tparam T The type of the uniform buffer.
  /// @param uniform_buffer The uniform buffer containing the data which
  /// will be accessed by the shader.
  /// @param binding The binding index which will be used in the SPIR-V shader.
  /// @param shader_stage The shader stage the uniform buffer will be used in,
  /// most likely the vertex shader.
  /// @return A reference to this DescriptorBuilder instance.
  template <typename T>
  DescriptorBuilder& addUniformBuffer(
    VkBuffer uniform_buffer, uint32_t binding,
    VkShaderStageFlagBits shader_stage = VK_SHADER_STAGE_VERTEX_BIT);

  /// @brief Adds a combined image sampler to the descriptor container.
  /// @param image_sampler The pointer to the combined image sampler.
  /// @param image_view The pointer to the image view.
  /// @param binding The binding index which will be used in the SPIR-V shader.
  /// @param shader_stage The shader stage the uniform buffer will be used in.
  /// @return A reference to this DescriptorBuilder instance.
  DescriptorBuilder& addCombinedImageSampler(
    VkSampler image_sampler, VkImageView image_view, uint32_t binding,
    VkShaderStageFlagBits shader_stage = VK_SHADER_STAGE_FRAGMENT_BIT);

  /// @brief Builds the resource descriptor.
  /// @param name The internal name of the resource descriptor.
  /// @return The resource descriptor which was created by the builder.
  [[nodiscard]] ResourceDescriptor build(const std::string& name);

private:
  const Device& device_;

  std::vector<VkDescriptorSetLayoutBinding> layout_bindings_;
  std::vector<VkWriteDescriptorSet>         write_sets_;
  std::vector<VkDescriptorBufferInfo>       descriptor_buffer_infos_;
  std::vector<VkDescriptorImageInfo>        descriptor_image_infos_;
};

template <typename T>
DescriptorBuilder&
DescriptorBuilder::addUniformBuffer(VkBuffer uniform_buffer, uint32_t binding,
                                    VkShaderStageFlagBits shader_stage)
{
  assert(uniform_buffer);

  layout_bindings_.push_back({
    .binding            = binding,
    .descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
    .descriptorCount    = 1,
    .stageFlags         = static_cast<VkShaderStageFlags>(shader_stage),
    .pImmutableSamplers = nullptr,
  });

  descriptor_buffer_infos_.push_back({
    .buffer = uniform_buffer,
    .offset = 0,
    .range  = sizeof(T),
  });

  write_sets_.push_back(VkWriteDescriptorSet{
    .sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
    .pNext            = {},
    .dstSet           = nullptr,
    .dstBinding       = binding,
    .dstArrayElement  = 0,
    .descriptorCount  = 1,
    .descriptorType   = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
    .pImageInfo       = {},
    .pBufferInfo      = &descriptor_buffer_infos_.back(),
    .pTexelBufferView = {},
  });

  return *this;
}

} // namespace eldr::vk::wr
