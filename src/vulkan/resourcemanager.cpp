#include <eldr/vulkan/resourcemanager.hpp>
#include <eldr/vulkan/wrappers/sampler.hpp>

NAMESPACE_BEGIN(eldr::vk)

struct ResourceManager::Resources {
  wr::Sampler                     default_sampler;
  wr::Image                       white_image;
  wr::Image                       error_image;
  GltfMetallicRoughness*          default_metal_rough_material; // should own?
  std::unordered_set<wr::Sampler> samplers_;
};

ResourceManager::ResourceManager(const wr::Device& device) : device_(device) {

}
NAMESPACE_END(eldr::vk)
