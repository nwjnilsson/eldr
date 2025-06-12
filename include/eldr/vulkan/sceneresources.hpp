#pragma once
#include <eldr/vulkan/descriptorallocator.hpp>
#include <eldr/vulkan/fwd.hpp>
#include <eldr/vulkan/material.hpp>

#include <vector>

NAMESPACE_BEGIN(fastgltf)
struct Sampler;
struct Image;
struct Material;
struct Texture;
struct Asset;
NAMESPACE_END(fastgltf)

NAMESPACE_BEGIN(eldr::vk)

// If draw commands are queued up with resources from this struct, the data here
// must not be destroyed until the device is idle or not using the resources
// anymore. Responsibility should be placed on engine or delay destruction of
// scene resources some other way.
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
NAMESPACE_END(eldr::vk)
