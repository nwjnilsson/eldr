#pragma once
#include <eldr/core/config.hpp>
#include <eldr/render/fwd.hpp>
#include <eldr/vulkan/fwd.hpp>

#include <filesystem>
#include <string>
#include <unordered_map>

NAMESPACE_BEGIN(eldr)

class SceneManager {
  // Scene manager maintains scalar RGBA scene representations. The idea is to
  // convert to spectral or whatever else when starting the render
  using Float    = float;
  using Spectrum = Color<float, 4>;
  EL_IMPORT_TYPES(Scene, Mesh)

public:
  SceneManager();
  ~SceneManager() = default;

  [[nodiscard]] bool loadGltf(const vk::VulkanEngine& engine,
                              std::filesystem::path   file_path);

  [[nodiscard]]
  bool loadObj(std::filesystem::path file_path);

  [[nodiscard]]
  bool load(const vk::VulkanEngine& engine, const std::filesystem::path&);

  void                       setActiveScene(std::string_view name);
  [[nodiscard]] Scene*       getActiveScene() { return active_scene_; }
  [[nodiscard]] const Scene* getActiveScene() const { return active_scene_; }

private:
  Scene*                                 active_scene_{ nullptr };
  std::unordered_map<std::string, Scene> scenes_;
};
NAMESPACE_END(eldr)
