// Ensure that vma implementation is included
#define VMA_IMPLEMENTATION
#include <eldr/app/window.hpp>
#include <eldr/core/exceptions.hpp>
#include <eldr/core/math.hpp>
#include <eldr/core/platform.hpp>
#include <eldr/core/stopwatch.hpp>
#include <eldr/render/mesh.hpp>
#include <eldr/render/scene.hpp>
#include <eldr/vulkan/common.hpp>
#include <eldr/vulkan/engine.hpp>
#include <eldr/vulkan/imgui.hpp>
#include <eldr/vulkan/rendergraph.hpp>
#include <eldr/vulkan/vktypes.hpp>
#include <eldr/vulkan/wrappers/buffer.hpp>
#include <eldr/vulkan/wrappers/commandbuffer.hpp>
#include <eldr/vulkan/wrappers/debugutilsmessenger.hpp>
#include <eldr/vulkan/wrappers/descriptorallocator.hpp>
#include <eldr/vulkan/wrappers/descriptorsetlayoutbuilder.hpp>
#include <eldr/vulkan/wrappers/descriptorwriter.hpp>
#include <eldr/vulkan/wrappers/device.hpp>
#include <eldr/vulkan/wrappers/gputexture.hpp>
#include <eldr/vulkan/wrappers/instance.hpp>
#include <eldr/vulkan/wrappers/shader.hpp>
#include <eldr/vulkan/wrappers/surface.hpp>
#include <eldr/vulkan/wrappers/swapchain.hpp>
#include <engine_config.hpp>

#include <GLFW/glfw3.h>
#include <imgui.h>

#include <memory>
#include <string>

namespace eldr::vk {
struct GpuModelData {
  ELDR_IMPORT_CORE_TYPES()
  Mat4f model_mat;
};

struct GpuMeshBuffers {
  BufferResource* index_buffer;
  BufferResource* vertex_buffer;
  VkDeviceAddress vertex_buffer_address;
};

VulkanEngine::VulkanEngine(const Window& window)
  : window_(window), log_(requestLogger("vulkan-engine"))
{
  // ---------------------------------------------------------------------------
  // Create instance
  // ---------------------------------------------------------------------------
  const VkApplicationInfo app_info{
    .sType            = VK_STRUCTURE_TYPE_APPLICATION_INFO,
    .pNext            = {},
    .pApplicationName = app_name,
    .applicationVersion =
      VK_MAKE_API_VERSION(0, app_version[0], app_version[1], app_version[2]),
    .pEngineName   = engine_name,
    .engineVersion = VK_MAKE_API_VERSION(engine_version[0], engine_version[1],
                                         engine_version[2], engine_version[3]),
    .apiVersion    = VK_API_VERSION_1_3,
  };

  instance_ =
    std::make_unique<wr::Instance>(app_info, window_.instanceExtensions());

  // ---------------------------------------------------------------------------
  // Create debug messenger
  // ---------------------------------------------------------------------------
#ifdef ELDR_VULKAN_DEBUG_REPORT
  debug_messenger_ = std::make_unique<wr::DebugUtilsMessenger>(*instance_);
#endif
  // ---------------------------------------------------------------------------
  // Create surface
  // ---------------------------------------------------------------------------
  surface_ = std::make_unique<wr::Surface>(*instance_, window_.glfwWindow());
  // ---------------------------------------------------------------------------
  // Create device
  // ---------------------------------------------------------------------------
  std::vector<const char*> device_extensions;
  device_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
#ifdef VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME
  device_extensions.push_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
#endif
  // Pass log_ pointer to device so that all wrapper objects that have a
  // reference to the device can access the same logger
  device_ = std::make_unique<wr::Device>(*instance_, *surface_,
                                         device_extensions, log_);

  // ---------------------------------------------------------------------------
  // Create swapchain
  // ---------------------------------------------------------------------------
  swapchain_ = std::make_unique<wr::Swapchain>(
    *device_, *surface_, VkExtent2D{ window_.width(), window_.height() });
  // ---------------------------------------------------------------------------
  // Load textures and shaders
  // ---------------------------------------------------------------------------
  loadTextures();
  loadShaders();

  // ---------------------------------------------------------------------------
  // Set up frame data
  // ---------------------------------------------------------------------------
  setupFrameData();
  initDescriptors();
  //  ---------------------------------------------------------------------------
  //  Create render graph
  //  ---------------------------------------------------------------------------
  recreateSwapchain();
}

VulkanEngine::~VulkanEngine() { device_->waitIdle(); }

void VulkanEngine::loadTextures()
{
  log_->trace("Loading textures...");
  const char* env_p = std::getenv("ELDR_DIR");
  if (env_p == nullptr) {
    Throw("Environment not set up correctly");
  }
  // TODO: build a vector of texture filenames from some configuration and load
  // each texture
  const std::string     texture_path = "/assets/textures/viking_room.png";
  std::filesystem::path filepath(std::string(env_p) + texture_path);

  Bitmap bitmap(filepath);
  if (bitmap.pixelFormat() != Bitmap::PixelFormat::RGBA)
    bitmap.rgbToRgba();
  textures_.emplace_back(*device_, bitmap);
}

void VulkanEngine::loadShaders()
{
  // TODO: get a list of shaders to load from config or something and load all
  // of them into shaders_ vector
  shaders_.emplace_back(*device_, VK_SHADER_STAGE_VERTEX_BIT,
                        "default vertex shader", "main.vert.spv");
  shaders_.emplace_back(*device_, VK_SHADER_STAGE_FRAGMENT_BIT,
                        "default frag shader", "main.frag.spv");
}

void VulkanEngine::setupFrameData()
{
  for (uint8_t i = 0; i < max_frames_in_flight; ++i) {
    std::vector<wr::DescriptorAllocator::PoolSizeRatio> frame_sizes{
      { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3 },
      { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3 }
    };
    frames_in_flight_.push_back(
      { .descriptors = std::make_unique<wr::DescriptorAllocator>(*device_, 1000,
                                                                 frame_sizes),
        .scene_data_buffer = std::make_unique<wr::GpuBuffer>(
          *device_, sizeof(GpuSceneData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
          VMA_MEMORY_USAGE_CPU_TO_GPU, "Scene data uniform buffer"),
        .model_data_buffer = std::make_unique<wr::GpuBuffer>(
          *device_, sizeof(GpuModelData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
          VMA_MEMORY_USAGE_CPU_TO_GPU, "Model data uniform buffer"),
        .cmd_buf = nullptr });
  }
}

void VulkanEngine::initDescriptors()
{

  wr::DescriptorSetLayoutBuilder layout_builder;
  wr::DescriptorSetLayout        layout{
    layout_builder
      .addUniformBuffer(0, VK_SHADER_STAGE_VERTEX_BIT |
                                    VK_SHADER_STAGE_FRAGMENT_BIT)
      .addUniformBuffer(1, VK_SHADER_STAGE_VERTEX_BIT)
      .addCombinedImageSampler(2, VK_SHADER_STAGE_FRAGMENT_BIT)
      .build(*device_, 0)
  };

  gpu_scene_data_descriptor_layout =
    std::make_unique<wr::DescriptorSetLayout>(std::move(layout));
}

void VulkanEngine::buildBuffers()
{
  indices_.clear();
  vertices_.clear();

  std::unordered_map<GpuVertex, uint32_t> unique_vertices{};
  // Vertex deduplication
  size_t total_vtx_count{ 0 };
  for (auto& node : scene_nodes_) {
    if (auto* mesh_node{ dynamic_cast<MeshNode*>(node.get()) }) {
      const auto&  mesh{ mesh_node->mesh };
      const size_t vtx_count{ mesh->vtxPositions().size() };
      total_vtx_count += vtx_count;
      for (uint32_t i = 0; i < vtx_count; ++i) {
        GpuVertex v{ mesh->vtxPositions()[i], mesh->vtxTexCoords()[i],
                     mesh->vtxColors()[i], mesh->vtxNormals()[i] };
        if (unique_vertices.count(v) == 0) {
          unique_vertices[v] = static_cast<uint32_t>(vertices_.size());
          vertices_.push_back(v);
        }
        indices_.push_back(unique_vertices[v]);
      }
    }
  }
  log_->debug("Vertex deduplication before and after {} -> {}", total_vtx_count,
              vertices_.size());

  vertex_buffer_->uploadData<GpuVertex>(vertices_);
  index_buffer_->uploadData<uint32_t>(indices_);
}

void VulkanEngine::setupRenderGraph()
{
  // 1 sample for back buffer, max possible sample count for color images and
  // depth buffer.
  // TODO: may want to be more flexible when setting sample count.
  // Note that the sample count used for the depth and color buffer must match
  // `.rasterizationSamples` in VkPipelineMultisampleStateCreateInfo.
  const VkSampleCountFlagBits sample_count = device_->findMaxMsaaSampleCount();

  msaa_buffer_ = render_graph_->add<TextureResource>(
    "MSAA color buffer", TextureUsage::ColorBuffer, swapchain_->imageFormat(),
    sample_count);

  auto* depth_buffer = render_graph_->add<TextureResource>(
    "depth buffer", TextureUsage::DepthStencilBuffer,
    device_->findDepthFormat(), sample_count);

  // TODO: handle resolve buffers implicitly in render graph
  back_buffer_ = render_graph_->add<TextureResource>(
    "back buffer", TextureUsage::BackBuffer, swapchain_->imageFormat());

  index_buffer_ = render_graph_->add<BufferResource>("index buffer",
                                                     BufferUsage::IndexBuffer);
  index_buffer_->uploadData<uint32_t>(indices_);

  vertex_buffer_ = render_graph_->add<BufferResource>(
    "vertex buffer", BufferUsage::VertexBuffer);
  vertex_buffer_->addVertexAttribute(VK_FORMAT_R32G32B32_SFLOAT,
                                     offsetof(GpuVertex, pos));
  vertex_buffer_->addVertexAttribute(VK_FORMAT_R32G32_SFLOAT,
                                     offsetof(GpuVertex, uv));
  vertex_buffer_->addVertexAttribute(VK_FORMAT_R32G32B32A32_SFLOAT,
                                     offsetof(GpuVertex, color));
  vertex_buffer_->setElementSize(sizeof(GpuVertex));
  vertex_buffer_->uploadData<GpuVertex>(vertices_);

  auto* main_stage = render_graph_->add<GraphicsStage>("main stage");
  // Write order matters here (for now)
  main_stage->writesTo(msaa_buffer_); // clear value index 0 is clear color
  main_stage->writesTo(depth_buffer); // clear value index 1 is depth stencil
  main_stage->writesTo(back_buffer_);
  main_stage->readsFrom(index_buffer_);
  main_stage->readsFrom(vertex_buffer_);
  main_stage->bindBuffer(vertex_buffer_, 0);
  main_stage->setClearsScreen(true);
  main_stage->setDepthOptions(true, true);
  main_stage->setOnRecord(
    [&](const PhysicalStage& physical, const wr::CommandBuffer& cb) {
      drawGeometry(physical, cb);
    });

  for (const auto& shader : shaders_) {
    main_stage->usesShader(shader);
  }

  // TODO: Only one descriptor set is currently allocated for any given
  // ResourceDescriptor. Additionally, for each frame in flight, the
  // descriptor set bound is identical to the previous frame.This means that it
  // is (for now) okay to only add a single layout for each resource descriptor
  // that applies to all frames.
  // for (const auto& descriptor : descriptors_)
  main_stage->addDescriptorLayout(gpu_scene_data_descriptor_layout->get());
}

void VulkanEngine::recreateSwapchain()
{
  window_.waitForFocus();
  device_->waitIdle();
  swapchain_->setupSwapchain(VkExtent2D{ window_.width(), window_.height() });
  // TODO: experiment with render graph creation/compilation. It is not
  // necessary to rebuild the whole thing on every swapchain invalidation.
  render_graph_.reset();
  render_graph_ = std::make_unique<RenderGraph>(*device_, *swapchain_);
  setupRenderGraph();
  // Reset first to destroy ImGui context
  imgui_overlay_.reset();
  imgui_overlay_ = std::make_unique<ImGuiOverlay>(
    *device_, *swapchain_, render_graph_.get(), back_buffer_);
  render_graph_->compile();
}

void VulkanEngine::updateScene(uint32_t current_image)
{
  // TODO: if scene has changed, rebuild vertices/indices
  static size_t last_scene_node_count{ 0 };
  if (scene_nodes_.size() != last_scene_node_count) {
    buildBuffers();
    last_scene_node_count = scene_nodes_.size();
  }

  main_draw_context_.opaque_surfaces.clear();
  for (auto& node : scene_nodes_) {
    node->draw(Mat4f{ 1.f }, main_draw_context_);
  }

  static StopWatch stop_watch;
  float            time{ stop_watch.seconds(false) };

  Mat4f model{ glm::rotate(Mat4f(1.0f), time * glm::radians(20.0f),
                           Vec3f(0.0f, 0.0f, 1.0f)) };
  Mat4f view{ glm::lookAt(Vec3f(2.0f, 2.0f, 2.0f), Vec3f(0.0f, 0.0f, 0.0f),
                          Vec3f(0.0f, 0.0f, 1.0f)) };
  Mat4f proj{ glm::perspective(glm::radians(45.0f),
                               swapchain_->extent().width /
                                 (float) swapchain_->extent().height,
                               0.1f, 10.0f) };
  GpuModelData model_data{ .model_mat = model };
  proj[1][1] *= -1;
  scene_data_.proj               = proj;
  scene_data_.view               = view;
  scene_data_.viewproj           = proj * view;
  scene_data_.ambient_color      = {};
  scene_data_.sunlight_color     = {};
  scene_data_.sunlight_direction = {};
  frames_in_flight_[current_image].scene_data_buffer->uploadData(
    &scene_data_, sizeof(scene_data_));
  frames_in_flight_[current_image].model_data_buffer->uploadData(
    &model_data, sizeof(model_data));
}

void VulkanEngine::drawGeometry(const PhysicalStage&     physical,
                                const wr::CommandBuffer& cb)
{
  // for (const RenderObject& draw : main_draw_context_.opaque_surfaces) {
  const size_t surface_count{ main_draw_context_.opaque_surfaces.size() };
  size_t       idx_offset{ 0 };
  for (size_t i = 0; i < surface_count; ++i) {
    const RenderObject& draw{ main_draw_context_.opaque_surfaces[i] };
    // cb.bindDescriptorSets(descriptors_[frame_index_].descriptorSets(),
    //                       physical.pipelineLayout());
    VkDescriptorSet global_descriptor{
      frames_in_flight_[frame_index_].descriptors->allocate(
        gpu_scene_data_descriptor_layout->get())
    };

    wr::DescriptorWriter writer;
    writer.writeUniformBuffer<GpuSceneData>(
      0, frames_in_flight_[frame_index_].scene_data_buffer->get());
    writer.writeUniformBuffer<GpuModelData>(
      1, frames_in_flight_[frame_index_].model_data_buffer->get());
    writer.writeCombinedImageSampler(2, viking_texture_->imageView(),
                                     viking_texture_->sampler());
    writer.updateSet(*device_, global_descriptor);

    // cb.bindDescriptorSets(draw.material->descriptor.descriptorSets(),
    // physical.pipelineLayout(), 1);

    const VkViewport viewports[] = { {
      .x        = 0.0f,
      .y        = 0.0f,
      .width    = static_cast<float>(swapchain_->extent().width),
      .height   = static_cast<float>(swapchain_->extent().height),
      .minDepth = 0.0f,
      .maxDepth = 1.0f,
    } };
    cb.setViewport(viewports, 0);

    const VkRect2D scissors[] = { {
      .offset = { 0, 0 },
      .extent = swapchain_->extent(),
    } };
    cb.setScissor(scissors, 0);

    // TODO: push constants

    cb.drawIndexed(draw.index_count, 1, draw.first_index + idx_offset);
    idx_offset += draw.index_count;
  }
}

void VulkanEngine::updateImGui(std::function<void()> const& lambda)
{
  ImGuiIO& io    = ImGui::GetIO();
  io.DisplaySize = ImVec2(static_cast<float>(swapchain_->extent().width),
                          static_cast<float>(swapchain_->extent().height));
  io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
  ImGui::NewFrame();
  lambda();
  ImGui::EndFrame();
  ImGui::Render();
  imgui_overlay_->update(frame_index_);
}

// TODO: move this to where it is relevant
void VulkanEngine::uploadMesh(std::span<const Point3f> positions,
                              std::span<const Vec2f>   texcoords,
                              std::span<const Color4f> colors,
                              std::span<const Vec3f>   normals)
{
  vertices_.clear();
  indices_.clear();

  // TODO:
  // De-duplication should be taken care of earlier, e.g when loading mesh
  // uploadMesh should look up the mesh's vertex/index buffer resource and do
  // resource->uploadData<GpuVertex>(vertices) (though this requires a vector of
  // GpuVertex...)
  std::unordered_map<GpuVertex, uint32_t> unique_vertices{};
  // Vertex deduplication
  for (uint32_t i = 0; i < positions.size(); ++i) {
    GpuVertex v{ positions[i], texcoords[i], colors[i], normals[i] };
    if (unique_vertices.count(v) == 0) {
      unique_vertices[v] = static_cast<uint32_t>(vertices_.size());
      vertices_.push_back(v);
    }
    indices_.push_back(unique_vertices[v]);
  }
  log_->debug("Vertex deduplication before and after {} -> {}",
              positions.size(), vertices_.size());
  // Recreate swapchain to trigger recompilation of render graph with new
  // vertex/index buffers. TODO: improve
  recreateSwapchain();
}

void VulkanEngine::drawFrame()
{
  if (swapchain_invalidated_) {
    recreateSwapchain();
    swapchain_invalidated_ = false;
    return;
  }

  updateScene(frame_index_); // move

  FrameData& frame{ frames_in_flight_[frame_index_] };

  // Wait until the previous command buffer with the current frame index
  // has finished executing
  if (likely(frame.cmd_buf != nullptr))
    frame.cmd_buf->waitFence();

  frame.descriptors->resetPools();

  const uint32_t image_index{ swapchain_->acquireNextImage(
    frame_index_, swapchain_invalidated_) };
  if (swapchain_invalidated_) {
    // Skip rendering, swapchain is recreated on next call
    return;
  }

  const auto& cb{ device_->requestCommandBuffer() };
  frame.cmd_buf = &cb;
  render_graph_->render(image_index, cb);

  VkPipelineStageFlags wait_stages[]{
    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
  };

  const VkSubmitInfo submit_info{
    .sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO,
    .pNext                = {},
    .waitSemaphoreCount   = 1,
    .pWaitSemaphores      = swapchain_->imageAvailableSemaphore(frame_index_),
    .pWaitDstStageMask    = wait_stages,
    .commandBufferCount   = 1,
    .pCommandBuffers      = &cb.get(),
    .signalSemaphoreCount = 1,
    .pSignalSemaphores    = swapchain_->renderFinishedSemaphore(frame_index_),
  };

  // Submit without waiting (we wait at the beginning of this function)
  cb.submit(submit_info);

  VkSwapchainKHR         swapchains[]{ swapchain_->get() };
  const VkPresentInfoKHR present_info{
    .sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
    .pNext              = {},
    .waitSemaphoreCount = 1,
    .pWaitSemaphores    = swapchain_->renderFinishedSemaphore(frame_index_),
    .swapchainCount     = 1,
    .pSwapchains        = swapchains,
    .pImageIndices      = &image_index,
    .pResults           = {},
  };

  swapchain_->present(present_info, swapchain_invalidated_);

  frame_index_ = (frame_index_ + 1) % max_frames_in_flight;
}

std::string VulkanEngine::deviceName() const { return device_->name(); }

} // namespace eldr::vk
