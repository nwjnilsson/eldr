#pragma once
#include "eldr/vulkan/material.hpp"
#include <eldr/core/logger.hpp>
#include <eldr/render/scene.hpp>

#include <functional>
#include <span>

namespace eldr {
// fwd
class EldrApp;
class Window;
namespace vk {

// TODO: decide where to put this struct
struct FrameData {
  std::unique_ptr<DescriptorAllocator> descriptors;
  std::unique_ptr<wr::GpuBuffer>       scene_data_buffer;
  std::unique_ptr<wr::GpuBuffer>       model_data_buffer;
  const wr::CommandBuffer*             cmd_buf;
};

struct GpuSceneData {
  ELDR_IMPORT_CORE_TYPES()
  Mat4f view;
  Mat4f proj;
  Mat4f viewproj;
  Vec4f ambient_color;
  Vec4f sunlight_direction;
  Vec4f sunlight_color;
};

// TODO: move Vulkan Engine to eldr:: namespace?
class VulkanEngine {
  ELDR_IMPORT_CORE_TYPES();
  friend EldrApp; // TODO: I did this to be able to invalidate swapchain from
                  // EldrApp. This is probably not ideal and there should be a
                  // better way to do this

public:
  VulkanEngine() = delete;
  VulkanEngine(const Window& window);
  ~VulkanEngine();

  // template <typename T, typename... Args> T* add(Args&&... args)
  //{
  //   auto ptr = std::make_unique<T>(std::forward<Args>(args)...);
  //   if constexpr (std::is_same_v<T, SceneNode>) {
  //     return
  //     static_cast<T*>(scene_nodes_.emplace_back(std::move(ptr)).get());
  //   }
  //   else {
  //     static_assert(!std::is_same_v<T, T>, "T must be a SceneNode ");
  //   }
  // }

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

  // TODO: put data in engine data, get rid of unique_ptrs
  struct EngineResources;
  std::unique_ptr<EngineResources> ed;

  DrawContext                             main_draw_context_;
  std::vector<std::shared_ptr<SceneNode>> scene_nodes_;

  std::unique_ptr<wr::Instance>            instance_;
  std::unique_ptr<wr::DebugUtilsMessenger> debug_messenger_;
  std::unique_ptr<wr::Surface>             surface_;
  std::unique_ptr<wr::Device>              device_;
  std::unique_ptr<wr::Swapchain>           swapchain_;
  std::unique_ptr<RenderGraph>             render_graph_;
  std::unique_ptr<ImGuiOverlay>            imgui_overlay_;
  std::vector<wr::GpuTexture>              textures_;
  std::vector<GpuVertex>                   vertices_;
  std::vector<uint32_t>                    indices_;
  std::vector<wr::Shader> shaders_; // shader module is not needed after
                                    // building pipeline so check if this can be
                                    // rearranged
  TextureResource*       back_buffer_{ nullptr };
  TextureResource*       msaa_buffer_{ nullptr };
  BufferResource*        vertex_buffer_{ nullptr };
  BufferResource*        index_buffer_{ nullptr };
  std::vector<FrameData> frames_in_flight_;

  // The data below is experimental, default data
  std::unique_ptr<wr::GpuTexture>          default_texture_sampler_linear_;
  GpuSceneData                             scene_data_;
  std::unique_ptr<wr::DescriptorSetLayout> gpu_scene_data_descriptor_layout_;
  std::unique_ptr<wr::DescriptorSetLayout> viking_model_descriptor_layout_;
  MaterialInstance                         default_data_;
  GltfMetallicRoughness                    metal_rough_material_;

  std::unique_ptr<wr::GpuTexture> viking_texture_;
  // std::unordered_map<Mesh*, GpuMeshBuffers> mesh_buffer_table_;
};

} // namespace vk
} // namespace eldr
