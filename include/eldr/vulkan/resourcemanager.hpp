#pragma once
#include <eldr/vulkan/descriptorallocator.hpp>
#include <eldr/vulkan/fwd.hpp>
#include <eldr/vulkan/material.hpp>

#include <filesystem>

NAMESPACE_BEGIN(fastgltf)
struct Sampler;
struct Image;
struct Material;
struct Texture;
struct Asset;
NAMESPACE_END(fastgltf)

NAMESPACE_BEGIN(eldr::vk)

class ResourceManager {
  friend VulkanEngine;

public:
  ResourceManager(const wr::Device&       device,
                  GltfMetallicRoughness&& default_material);
  ResourceManager(ResourceManager&&) noexcept;
  ~ResourceManager();

  ResourceManager& operator=(ResourceManager&&) noexcept;

  [[nodiscard]] std::shared_ptr<wr::Image> const errorImage() const;

  [[nodiscard]] std::shared_ptr<wr::Sampler> const defaultSampler() const;

  [[nodiscard]] std::optional<std::shared_ptr<wr::Image>>
  loadImage(const fastgltf::Asset&       asset,
            fastgltf::Image&             image,
            const std::filesystem::path& texture_dir);

  void load(fastgltf::Asset& asset);

  // void load(fastgltf::Asset&);
  // void buildMaterialPipelines(GltfMetallicRoughness& material);

private:
  const wr::Device* device_{ nullptr };
  struct Resources;
  std::unique_ptr<Resources> d_;
};
NAMESPACE_END(eldr::vk)
