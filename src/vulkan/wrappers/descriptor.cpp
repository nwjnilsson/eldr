#include <eldr/core/logger.hpp>
#include <eldr/vulkan/wrappers/descriptor.hpp>
#include <eldr/vulkan/wrappers/descriptorpool.hpp>
#include <eldr/vulkan/wrappers/device.hpp>

namespace eldr::vk::wr {
ResourceDescriptor::ResourceDescriptor(
  Device& device, std::vector<VkDescriptorSetLayoutBinding>&& bindings,
  std::vector<VkWriteDescriptorSet>&& descriptor_writes)
  : device_(device)
{
  // Create descriptor pool
  /**
   * From Vulkan tutorial:
   *
   * Inadequate descriptor pools are a good example of a problem that the
   * validation layers will not catch: As of Vulkan 1.1,
   * vkAllocateDescriptorSets may fail with the error code
   * VK_ERROR_POOL_OUT_OF_MEMORY if the pool is not sufficiently large, but the
   * driver may also try to solve the problem internally. This means that
   * sometimes (depending on hardware, pool size and allocation size) the driver
   * will let us get away with an allocation that exceeds the limits of our
   * descriptor pool. Other times, vkAllocateDescriptorSets will fail and return
   * VK_ERROR_POOL_OUT_OF_MEMORY. This can be particularly frustrating if the
   * allocation succeeds on some machines, but fails on others.
   */
  std::vector<VkDescriptorPoolSize> pool_sizes{
    { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
      static_cast<uint32_t>(max_frames_in_flight) },
    { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
      static_cast<uint32_t>(max_frames_in_flight) },
    { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 }
  }; // ImGui

  auto pool_ci          = makeInfo<VkDescriptorPoolCreateInfo>();
  pool_ci.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
  pool_ci.pPoolSizes    = pool_sizes.data();
  pool_ci.maxSets       = static_cast<uint32_t>(max_frames_in_flight) + 1;
  VkResult result = vkCreateDescriptorPool(device_.logical(), &pool_ci, nullptr,
                                           &descriptor_pool_);
  CheckVkResult(result);

  // Create layout
  auto layout_ci         = makeInfo<VkDescriptorSetLayoutCreateInfo>();
  layout_ci.bindingCount = static_cast<uint32_t>(bindings.size());
  layout_ci.pBindings    = bindings.data();
  result = vkCreateDescriptorSetLayout(device_.logical(), &layout_ci, nullptr,
                                       &descriptor_set_layout_);
  CheckVkResult(result);

  std::vector<VkDescriptorSetLayout> layouts(max_frames_in_flight,
                                             descriptor_set_layout_);

  // Create descriptor sets
  auto alloc_info               = makeInfo<VkDescriptorSetAllocateInfo>();
  alloc_info.descriptorPool     = descriptor_pool_;
  alloc_info.descriptorSetCount = 1;
  alloc_info.pSetLayouts        = layouts.data();
  result = vkAllocateDescriptorSets(device_.logical(), &alloc_info,
                                    descriptor_sets_.data());
  CheckVkResult(result);

  for (size_t i = 0; i < descriptor_writes.size(); ++i) {
    descriptor_writes[i].dstSet     = descriptor_sets_[0];
    descriptor_writes[i].dstBinding = i;

    vkUpdateDescriptorSets(device_.logical(),
                           static_cast<uint32_t>(descriptor_writes.size()),
                           descriptor_writes.data(), 0, nullptr);
  }
}

ResourceDescriptor::~ResourceDescriptor()
{
  if (descriptor_set_layout_ != VK_NULL_HANDLE)
    vkDestroyDescriptorSetLayout(device_.logical(), descriptor_set_layout_,
                                 nullptr);

  if (descriptor_pool_ != VK_NULL_HANDLE)
    vkDestroyDescriptorPool(device_.logical(), descriptor_pool_, nullptr);
}
} // namespace eldr::vk::wr
