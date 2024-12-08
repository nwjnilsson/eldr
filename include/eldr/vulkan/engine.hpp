#pragma once
#include <eldr/core/logger.hpp>
#include <eldr/render/scene.hpp>
#include <eldr/vulkan/fwd.hpp>

#include <functional>
#include <span>

// fwd declarations
namespace eldr {
class EldrApp;
class Window;
} // namespace eldr
namespace eldr::vk {
class VulkanEngine;
} // namespace eldr::vk

// TODO: move Vulkan Engine to eldr:: namespace?
namespace eldr::vk {
struct GpuVertex {
  ELDR_IMPORT_CORE_TYPES();
  Point3f pos;
  float   uv_x;
  Vec3f   normal;
  float   uv_y;
  Color4f color;
};

class VulkanEngine {
  ELDR_IMPORT_CORE_TYPES();
  friend EldrApp; // TODO: I did this to be able to invalidate swapchain from
                  // EldrApp. This is probably not ideal and there should be a
                  // better way to do this

public:
  VulkanEngine() = delete;
  VulkanEngine(const Window& window);
  ~VulkanEngine();

  void addSceneNode(std::shared_ptr<SceneNode> node)
  {
    scene_nodes_.push_back(node);
  }

  void updateImGui(std::function<void()> const& lambda);

  void uploadMesh(std::span<const Point3f> positions,
                  std::span<const Point2f> texcoords,
                  std::span<const Color4f> colors,
                  std::span<const Vec3f>   normals);
  void drawFrame();

  [[nodiscard]] std::string deviceName() const;

  void buildMaterialPipelines(GltfMetallicRoughness& material);

private:
  void loadTextures();
  void loadShaders();
  void setupFrameData();
  void initDescriptors();
  void initDefaultData();
  void createCommandBuffers();
  void setupRenderGraph();
  void recreateSwapchain();
  void buildBuffers();
  void updateScene(uint32_t current_image);
  void drawGeometry(const PhysicalStage& physical, const wr::CommandBuffer& cb);

private:
  const Window& window_;
  Logger        log_{ requestLogger("vulkan-engine") };

  bool     initialized_{ false };
  bool     swapchain_invalidated_{ false };
  uint32_t frame_index_{ 0 };

  std::vector<GpuVertex> vertices_;
  std::vector<uint32_t>  indices_;

  TextureResource* back_buffer_;
  TextureResource* msaa_buffer_;
  BufferResource*  vertex_buffer_;
  BufferResource*  index_buffer_;

  // Hide vulkan implementation details to avoid pulling in every single vulkan
  // related type when including engine.hpp
  struct EngineData;
  std::unique_ptr<EngineData> ed_;

  struct GpuSceneData {
    ELDR_IMPORT_CORE_TYPES()
    Mat4f view;
    Mat4f proj;
    Mat4f viewproj;
    Vec4f ambient_color;
    Vec4f sunlight_direction;
    Vec4f sunlight_color;
  } scene_data_;

  // TODO: decide whether these go in EngineData
  DrawContext                             main_draw_context_;
  std::vector<std::shared_ptr<SceneNode>> scene_nodes_;

  // std::unordered_map<Mesh*, GpuMeshBuffers> mesh_buffer_table_;
};

} // namespace eldr::vk
namespace std {
template <> struct hash<eldr::vk::GpuVertex> {
  ELDR_IMPORT_CORE_TYPES();
  size_t operator()(eldr::vk::GpuVertex const& vertex) const;
};
} // namespace std
