#include <eldr/core/logger.hpp>
#include <eldr/vulkan/wrappers/descriptor.hpp>
#include <eldr/vulkan/wrappers/descriptorpool.hpp>
#include <eldr/vulkan/wrappers/device.hpp>

namespace eldr::vk::wr {
ResourceDescriptor::ResourceDescriptor(
  const Device& device, std::vector<VkDescriptorSetLayoutBinding>&& bindings,
  std::vector<VkWriteDescriptorSet>&& descriptor_writes,
  const std::string&                  name)
  : device_(device), name_(name), bindings_(bindings),
    descriptor_writes_(descriptor_writes)
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

  const VkDescriptorPoolCreateInfo pool_ci{
    .sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
    .pNext         = {},
    .flags         = {},
    .maxSets       = static_cast<uint32_t>(max_frames_in_flight) + 1,
    .poolSizeCount = static_cast<uint32_t>(pool_sizes.size()),
    .pPoolSizes    = pool_sizes.data(),
  };

  if (const VkResult result = vkCreateDescriptorPool(
        device_.logical(), &pool_ci, nullptr, &descriptor_pool_);
      result != VK_SUCCESS)
    ThrowVk(result, "vkCreateDescriptorPool(): ");

  // Create layout
  const VkDescriptorSetLayoutCreateInfo layout_ci{
    .sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
    .pNext        = {},
    .flags        = {},
    .bindingCount = static_cast<uint32_t>(bindings_.size()),
    .pBindings    = bindings_.data(),
  };

  if (const VkResult result = vkCreateDescriptorSetLayout(
        device_.logical(), &layout_ci, nullptr, &descriptor_set_layout_);
      result != VK_SUCCESS)
    ThrowVk(result, "vkCreateDescriptorSetLayout(): ");

  std::vector<VkDescriptorSetLayout> layouts(max_frames_in_flight,
                                             descriptor_set_layout_);

  // Create descriptor sets
  const VkDescriptorSetAllocateInfo alloc_info{
    .sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
    .pNext              = {},
    .descriptorPool     = descriptor_pool_,
    .descriptorSetCount = 1,
    .pSetLayouts        = layouts.data(),
  };
  descriptor_sets_.resize(1);
  if (const auto result = vkAllocateDescriptorSets(
        device_.logical(), &alloc_info, descriptor_sets_.data());
      result != VK_SUCCESS)
    ThrowVk(result, "vkAllocateDescriptorSets(): ");

  for (size_t i = 0; i < descriptor_writes_.size(); ++i) {
    descriptor_writes_[i].dstSet     = descriptor_sets_[0];
    descriptor_writes_[i].dstBinding = static_cast<uint32_t>(i);
  }
  vkUpdateDescriptorSets(device_.logical(),
                         static_cast<uint32_t>(descriptor_writes_.size()),
                         descriptor_writes_.data(), 0, nullptr);
}

ResourceDescriptor::ResourceDescriptor(ResourceDescriptor&& other) noexcept
  : device_(other.device_), bindings_(std::move(other.bindings_)),
    descriptor_sets_(std::move(other.descriptor_sets_))
{
  name_            = std::exchange(other.name_, "");
  descriptor_pool_ = std::exchange(other.descriptor_pool_, VK_NULL_HANDLE);
  descriptor_set_layout_ =
    std::exchange(other.descriptor_set_layout_, VK_NULL_HANDLE);
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
