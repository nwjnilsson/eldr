#pragma once
#include <eldr/core/logger.hpp>
#include <eldr/render/scene.hpp>

#include <functional>
#include <span>

namespace eldr {
// fwd
class EldrApp;
class Window;
namespace vk {

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

private:
  void loadTextures();
  void loadShaders();
  void createUniformBuffers();
  void createCommandBuffers();
  void createResourceDescriptors();
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
  uint32_t current_frame_{ 0 };

  DrawContext                             main_draw_context_{};
  std::vector<std::shared_ptr<SceneNode>> scene_nodes_{};

  std::unique_ptr<wr::Instance>             instance_;
  std::unique_ptr<wr::DebugUtilsMessenger>  debug_messenger_;
  std::unique_ptr<wr::Surface>              surface_;
  std::unique_ptr<wr::Device>               device_;
  std::unique_ptr<wr::Swapchain>            swapchain_;
  std::vector<wr::ResourceDescriptor>       descriptors_;
  std::unique_ptr<RenderGraph>              render_graph_;
  std::unique_ptr<ImGuiOverlay>             imgui_overlay_;
  std::vector<wr::GpuTexture>               textures_;
  std::vector<GpuVertex>                    vertices_;
  std::vector<uint32_t>                     indices_;
  std::vector<wr::Shader>                   shaders_;
  TextureResource*                          back_buffer_{ nullptr };
  TextureResource*                          msaa_buffer_{ nullptr };
  BufferResource*                           vertex_buffer_{ nullptr };
  BufferResource*                           index_buffer_{ nullptr };
  std::vector<wr::GpuBuffer>                uniform_buffers_;
  std::vector<const wr::CommandBuffer*>     in_flight_cmd_bufs_;
  std::unique_ptr<wr::GpuTexture>           viking_texture_;
  std::unordered_map<Mesh*, GpuMeshBuffers> mesh_buffer_table_;
};

} // namespace vk
} // namespace eldr
