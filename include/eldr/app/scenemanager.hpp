#pragma once
#include <eldr/render/fwd.hpp>
#include <eldr/vulkan/fwd.hpp>

#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
NAMESPACE_BEGIN(eldr::app)

class SceneManager {
  EL_IMPORT_CORE_TYPES_SCALAR()
public:
  SceneManager();
  ~SceneManager();

  EL_VARIANT
  std::optional<std::vector<render::Mesh<Float, Spectrum>>>
  loadGltf(const vk::VulkanEngine& engine, std::filesystem::path file_path);

  EL_VARIANT [[nodiscard]]
  static std::optional<std::shared_ptr<render::Scene<Float, Spectrum>>>
  loadObj(std::filesystem::path file_path);

  EL_VARIANT [[nodiscard]]
  static std::optional<std::shared_ptr<render::Scene<Float, Spectrum>>>
  load(const vk::VulkanEngine& engine, const std::filesystem::path&);

  void                                   setActiveScene(std::string_view name);
  [[nodiscard]] render::SceneBase*       activeScene() { return active_scene_; }
  [[nodiscard]] const render::SceneBase* activeScene() const
  {
    return active_scene_;
  }

private:
  render::SceneBase*                                 active_scene_;
  std::unordered_map<std::string, render::SceneBase> scenes_;
  std::unique_ptr<vk::SceneResources>                vk_resources_;
};
NAMESPACE_END(eldr::app)
