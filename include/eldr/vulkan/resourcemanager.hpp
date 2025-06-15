#pragma once
#include <eldr/vulkan/fwd.hpp>

#include <filesystem>
#include <optional>
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
  friend VulkanEngine;

public:
  ResourceManager();
  ResourceManager(const wr::Device&       device,
                  GltfMetallicRoughness&& default_material);
  ResourceManager(ResourceManager&&) noexcept;
  ~ResourceManager();

  ResourceManager& operator=(ResourceManager&&);

  [[nodiscard]] const wr::Image& errorImage() const;

  [[nodiscard]] const wr::Sampler& defaultSampler() const;

  [[nodiscard]] std::optional<const wr::Image*>
  loadImage(const fastgltf::Asset&       asset,
            fastgltf::Image&             image,
            const std::filesystem::path& texture_dir);

  [[nodiscard]] std::vector<const Material*> load(fastgltf::Asset& asset);

  // void load(fastgltf::Asset&);
  // void buildMaterialPipelines(GltfMetallicRoughness& material);

private:
  const wr::Device* device_{ nullptr };
  struct Resources;
  std::unique_ptr<Resources> d_;
};
NAMESPACE_END(eldr::vk)
