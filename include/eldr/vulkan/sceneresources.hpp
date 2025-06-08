#pragma once
#include <eldr/vulkan/descriptorallocator.hpp>
#include <eldr/vulkan/fwd.hpp>
#include <eldr/vulkan/material.hpp>

#include <vector>

namespace fastgltf {
struct Sampler;
struct Image;
struct Material;
struct Texture;
struct Asset;
} // namespace fastgltf

namespace eldr::vk {

struct SceneResources {
  struct DefaultResources {
    const wr::Sampler*     sampler;
    const wr::Image*       white_image;
    const wr::Image*       error_image;
    GltfMetallicRoughness* metal_rough_material;
  };

  SceneResources(const wr::Device& device,
                 fastgltf::Asset&,
                 const DefaultResources&);
  ~SceneResources();

  DescriptorAllocator      material_descriptors;
  std::vector<wr::Sampler> samplers;
  std::vector<wr::Image>   images;
  std::vector<size_t>      image_indices;
  // std::unordered_map<std::string, std::shared_ptr<Material>> materials_;
  std::vector<std::shared_ptr<Material>>                   materials;
  vk::wr::Buffer<GltfMetallicRoughness::MaterialConstants> material_buffer;
};
} // namespace eldr::vk
