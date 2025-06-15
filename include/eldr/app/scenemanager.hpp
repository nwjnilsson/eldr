#pragma once
#include <eldr/render/fwd.hpp>
#include <eldr/vulkan/fwd.hpp>

#include <filesystem>
#include <string>
#include <unordered_map>

NAMESPACE_BEGIN(eldr)

class SceneManager {
  EL_IMPORT_CORE_TYPES_SCALAR()
public:
  SceneManager();
  ~SceneManager() = default;

  bool loadGltf(const vk::VulkanEngine& engine,
                std::filesystem::path   file_path);

  [[nodiscard]]
  bool loadObj(std::filesystem::path file_path);

  [[nodiscard]]
  bool load(const vk::VulkanEngine& engine, const std::filesystem::path&);

  void setActiveScene(std::string_view name);
  [[nodiscard]] Scene<float, Color<float, 3>>* activeScene()
  {
    return active_scene_;
  }
  [[nodiscard]] const Scene<float, Color<float, 3>>* activeScene() const
  {
    return active_scene_;
  }

private:
  Scene<float, Color<float, 3>>* active_scene_{ nullptr };
  std::unordered_map<std::string, Scene<float, Color<float, 3>>> scenes_;
};
NAMESPACE_END(eldr)
