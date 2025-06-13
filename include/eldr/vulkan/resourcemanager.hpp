#pragma once
#include <eldr/vulkan/descriptorallocator.hpp>
#include <eldr/vulkan/fwd.hpp>
#include <eldr/vulkan/material.hpp>

#include <unordered_set>
#include <vector>

NAMESPACE_BEGIN(fastgltf)
struct Sampler;
struct Image;
struct Material;
struct Texture;
struct Asset;
NAMESPACE_END(fastgltf)

NAMESPACE_BEGIN(eldr::vk)

class ResourceManager {

  ResourceManager(const wr::Device& device);
  ~ResourceManager();

  void load(fastgltf::Asset&);

private:
  const wr::Device& device_;
  struct Resources;
  std::unique_ptr<Resources> default_data_;

  DescriptorAllocator material_descriptors_;
  // std::vector<wr::Sampler> samplers_;
  std::vector<wr::Image> images_;
  std::vector<size_t>    image_indices_;
  // std::unordered_map<std::string, std::shared_ptr<Material>> materials_;
  std::vector<std::shared_ptr<Material>>                   materials_;
  vk::wr::Buffer<GltfMetallicRoughness::MaterialConstants> material_buffer_;
};
NAMESPACE_END(eldr::vk)
