#pragma once
#include <eldr/app/fwd.hpp>
#include <eldr/core/math.hpp>
#include <eldr/render/scene.hpp>
#include <eldr/vulkan/fwd.hpp>

#include <functional>
#include <span>

// fwd declarations
namespace eldr {
class EldrApp;
} // namespace eldr

// TODO: move Vulkan Engine to eldr:: namespace?
namespace eldr::vk {
// TODO: decide where structs should live
struct GpuSceneData {
  ELDR_IMPORT_CORE_TYPES()
  Mat4f view;
  Mat4f proj;
  Mat4f viewproj;
  Vec4f ambient_color;
  Vec4f sunlight_direction;
  Vec4f sunlight_color;
};
struct GpuModelData {
  ELDR_IMPORT_CORE_TYPES()
  Mat4f model_mat;
};

struct GpuVertex {
  ELDR_IMPORT_CORE_TYPES();
  Point3f pos;
  float   uv_x;
  Vec3f   normal;
  float   uv_y;
  Color4f color;

  bool operator==(const GpuVertex&) const = default;
};

class VulkanEngine {
  ELDR_IMPORT_CORE_TYPES();
  friend EldrApp; // TODO: I did this to be able to invalidate swapchain from
                  // EldrApp. This is probably not ideal and there should be a
                  // better way to do this

public:
  VulkanEngine() = delete;
  VulkanEngine(const app::Window& window);
  ~VulkanEngine();

  const wr::Device& device() const; // TODO: refactor and remove
  void addScene(const std::string& name, const std::shared_ptr<Scene>& scene)
  {
    loaded_scenes_[name] = scene;
  };

  GltfMetallicRoughness& metalRoughMaterial() const;
  const wr::Image&       whiteImage() const;
  const wr::Image&       errorImage() const;
  const wr::Sampler&     defaultSamplerLinear() const;

  void updateImGui(std::function<void()> const& lambda);

  void uploadMesh(std::span<const Point3f> positions,
                  std::span<const Point2f> texcoords,
                  std::span<const Color4f> colors,
                  std::span<const Vec3f>   normals);
  void drawFrame();

  [[nodiscard]] std::string deviceName() const;

  void buildMaterialPipelines(GltfMetallicRoughness& material);

  // TODO: decide how to handle this
  void invalidateSwapchain() { swapchain_invalidated_ = true; }

private:
  void loadTextures();
  void loadShaders();
  void setupFrameData();
  void initDescriptors();
  void initDefaultData();
  void createCommandBuffers();
  void setupRenderGraph();
  void recreateSwapchain();
  void updateBuffers();
  void updateScenes(uint32_t current_image);
  void drawGeometry(const wr::CommandBuffer& cb);

private:
  const app::Window& window_;

  bool     initialized_{ false };
  bool     swapchain_invalidated_{ false };
  uint32_t frame_index_{ 0 };

  // Hide vulkan implementation details to avoid pulling in every single vulkan
  // related type when including engine.hpp
  struct EngineData;
  std::unique_ptr<EngineData> d_;
  struct Settings;
  std::unique_ptr<Settings> s_;

  std::unordered_map<std::string, std::shared_ptr<Scene>> loaded_scenes_;

  GpuSceneData scene_data_;

  // TODO: decide whether these go in EngineData
  DrawContext main_draw_context_;

  // std::unordered_map<Mesh*, GpuMeshBuffers> mesh_buffer_table_;
};

} // namespace eldr::vk
template <> struct std::hash<eldr::vk::GpuVertex> {
  ELDR_IMPORT_CORE_TYPES();
  size_t operator()(eldr::vk::GpuVertex const& vertex) const;
};
