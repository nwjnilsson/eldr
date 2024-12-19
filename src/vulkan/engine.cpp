// Ensure that vma implementation is included
#define VMA_IMPLEMENTATION
#include <eldr/app/window.hpp>
#include <eldr/core/bitmap.hpp>
#include <eldr/core/exceptions.hpp>
#include <eldr/core/hash.hpp>
#include <eldr/core/math.hpp>
#include <eldr/core/platform.hpp>
#include <eldr/core/stopwatch.hpp>
#include <eldr/render/mesh.hpp>
#include <eldr/render/scene.hpp>
#include <eldr/vulkan/common.hpp>
#include <eldr/vulkan/descriptorallocator.hpp>
#include <eldr/vulkan/descriptorsetlayoutbuilder.hpp>
#include <eldr/vulkan/descriptorwriter.hpp>
#include <eldr/vulkan/engine.hpp>
#include <eldr/vulkan/imgui.hpp>
#include <eldr/vulkan/material.hpp>
#include <eldr/vulkan/pipelinebuilder.hpp>
#include <eldr/vulkan/rendergraph.hpp>
#include <eldr/vulkan/vktypes.hpp>
#include <eldr/vulkan/wrappers/buffer.hpp>
#include <eldr/vulkan/wrappers/commandbuffer.hpp>
#include <eldr/vulkan/wrappers/debugutilsmessenger.hpp>
#include <eldr/vulkan/wrappers/device.hpp>
#include <eldr/vulkan/wrappers/instance.hpp>
#include <eldr/vulkan/wrappers/sampler.hpp>
#include <eldr/vulkan/wrappers/shader.hpp>
#include <eldr/vulkan/wrappers/surface.hpp>
#include <eldr/vulkan/wrappers/swapchain.hpp>
#include <eldr/vulkan/wrappers/texture.hpp>
#include <engine_config.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <GLFW/glfw3.h>
#include <imgui.h>

#include <memory>
#include <string>
namespace eldr::vk {
// -----------------------------------------------------------------------------
// Engine types
// -----------------------------------------------------------------------------
struct FrameData {
  DescriptorAllocator      descriptors;
  wr::Buffer<GpuSceneData> scene_data_buffer;
  wr::Buffer<GpuModelData> model_data_buffer;
  const wr::CommandBuffer* cmd_buf;
};

// TODO: is this even used
struct GpuMeshBuffers {
  BufferResource* index_buffer_;
  BufferResource* vertex_buffer_;
  VkDeviceAddress vertex_buffer_address;
};

struct VulkanEngine::EngineData {
  wr::Instance            instance;
  wr::DebugUtilsMessenger debug_messenger;
  wr::Surface             surface;
  wr::Device              device;
  wr::Swapchain           swapchain;
  DescriptorAllocator     global_descriptor_allocator;

  std::unique_ptr<RenderGraph>  render_graph;
  std::unique_ptr<ImGuiOverlay> imgui_overlay;
  std::vector<wr::Texture>      textures;
  std::vector<wr::Shader>       shaders; // shader module is not needed after
                                   // building pipeline so check if this can be
                                   // rearranged
  std::vector<FrameData> frames_in_flight;

  // The data below is experimental, default data
  wr::DescriptorSetLayout scene_data_descriptor_layout;
  wr::DescriptorSetLayout viking_model_descriptor_layout;
  GltfMetallicRoughness   metal_rough_material;
  MaterialInstance        default_material_data;

  wr::Texture white_texture;
  wr::Texture error_texture;

  wr::Texture viking_texture;
  wr::Sampler default_sampler_linear;
};

// -----------------------------------------------------------------------------
// Engine
// -----------------------------------------------------------------------------
VulkanEngine::VulkanEngine(const Window& window)
  : window_(window), log_(requestLogger("vulkan-engine")),
    ed_(std::make_unique<EngineData>())
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

  ed_->instance = wr::Instance{ app_info, window_.instanceExtensions() };

  // ---------------------------------------------------------------------------
  // Create debug messenger
  // ---------------------------------------------------------------------------
#ifdef ELDR_VULKAN_DEBUG_REPORT
  ed_->debug_messenger = wr::DebugUtilsMessenger{ ed_->instance };
#endif
  // ---------------------------------------------------------------------------
  // Create surface
  // ---------------------------------------------------------------------------
  ed_->surface = wr::Surface{ ed_->instance, window_.glfwWindow() };
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
  ed_->device =
    wr::Device{ ed_->instance, ed_->surface, device_extensions, log_ };

  // ---------------------------------------------------------------------------
  // Create swapchain
  // ---------------------------------------------------------------------------
  ed_->swapchain = wr::Swapchain(
    ed_->device, ed_->surface, VkExtent2D{ window_.width(), window_.height() });
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

  // ---------------------------------------------------------------------------
  // Init default data
  // ---------------------------------------------------------------------------
  initDefaultData();
  buildMaterialPipelines(ed_->metal_rough_material);
  //  ---------------------------------------------------------------------------
  //  Create render graph
  //  ---------------------------------------------------------------------------
  recreateSwapchain();
}

VulkanEngine::~VulkanEngine() { ed_->device.waitIdle(); }

// TODO: This is cursed and needs to be refactored
GltfMetallicRoughness& VulkanEngine::metalRoughMaterial() const
{
  return ed_->metal_rough_material;
}
const wr::Texture& VulkanEngine::whiteImage() const
{
  return ed_->white_texture;
}
const wr::Texture& VulkanEngine::errorImage() const
{
  return ed_->error_texture;
}
const wr::Sampler& VulkanEngine::defaultSamplerLinear() const
{
  return ed_->default_sampler_linear;
}
// -----------------------------------------------------

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
  ed_->textures.emplace_back(ed_->device, bitmap);
}

void VulkanEngine::loadShaders()
{
  // TODO: get a list of shaders to load from config or something and load all
  // of them into shaders vector
  ed_->shaders.emplace_back(ed_->device, "default vertex shader",
                            "main.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
  ed_->shaders.emplace_back(ed_->device, "default frag shader", "main.frag.spv",
                            VK_SHADER_STAGE_FRAGMENT_BIT);
}

void VulkanEngine::setupFrameData()
{
  for (uint8_t i = 0; i < max_frames_in_flight; ++i) {

    // TODO: I will need to come back to this eventually and understand how to
    // set this dynamically, I can't thik atm
    std::vector<DescriptorAllocator::PoolSizeRatio> frame_sizes{
      { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3 },
      { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3 }
    };

    ed_->frames_in_flight.push_back({
      .descriptors = DescriptorAllocator{ 1000, frame_sizes },
      .scene_data_buffer =
        wr::Buffer<GpuSceneData>{
          ed_->device,
          "Scene data uniform buffer",
          1,
          VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
          VMA_MEMORY_USAGE_CPU_TO_GPU,
        },
      .model_data_buffer =
        wr::Buffer<GpuModelData>{ ed_->device, "Model data uniform buffer", 1,
                                  VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                  VMA_MEMORY_USAGE_CPU_TO_GPU },
      .cmd_buf = nullptr, // Set later when drawing frames
    });
  }
}

void VulkanEngine::initDescriptors()
{

  DescriptorSetLayoutBuilder layout_builder;
  layout_builder.addUniformBuffer(0, VK_SHADER_STAGE_VERTEX_BIT |
                                       VK_SHADER_STAGE_FRAGMENT_BIT);
  ed_->scene_data_descriptor_layout = layout_builder.build(ed_->device, 0);

  layout_builder.reset();

  layout_builder.addUniformBuffer(0, VK_SHADER_STAGE_VERTEX_BIT)
    .addCombinedImageSampler(1, VK_SHADER_STAGE_FRAGMENT_BIT);
  ed_->viking_model_descriptor_layout = layout_builder.build(ed_->device, 0);
}

void VulkanEngine::initDefaultData()
{
  const wr::Device& device{ ed_->device };
  // Create default white texture
  ed_->white_texture = wr::Texture{ device, Bitmap::createDefaultWhite() };

  // Create default checkerboard error texture
  ed_->error_texture = wr::Texture{ device, Bitmap::createCheckerboard() };

  // Create default linear sampler
  ed_->default_sampler_linear =
    wr::Sampler{ device, VK_FILTER_LINEAR, VK_FILTER_LINEAR,
                 VK_SAMPLER_MIPMAP_MODE_LINEAR,
                 ed_->white_texture.mipLevels() };
}

void VulkanEngine::buildBuffers()
{
  indices_.clear();
  vertices_.clear();

  std::unordered_map<GpuVertex, uint32_t> unique_vertices{};
  // Vertex deduplication
  size_t total_vtx_count{ 0 };
  for (auto& es : loaded_scenes) {
    for (auto& en : es.second->nodes) {
      if (const auto& mesh_node{
            dynamic_cast<const MeshNode*>(en.second.get()) }) {
        const auto&  mesh{ mesh_node->mesh };
        const size_t vtx_count{ mesh->vtxPositions().size() };
        total_vtx_count += vtx_count;
        for (uint32_t i = 0; i < vtx_count; ++i) {
          float     uv_x{ mesh->vtxTexCoords()[i].x };
          float     uv_y{ mesh->vtxTexCoords()[i].y };
          GpuVertex v{ mesh->vtxPositions()[i], uv_x, mesh->vtxNormals()[i],
                       uv_y, mesh->vtxColors()[i] };
          if (unique_vertices.count(v) == 0) {
            unique_vertices[v] = static_cast<uint32_t>(vertices_.size());
            vertices_.push_back(v);
          }
          indices_.push_back(unique_vertices[v]);
        }
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
  const VkSampleCountFlagBits sample_count{
    ed_->device.findMaxMsaaSampleCount()
  };

  VkFormat color_format{ ed_->swapchain.imageFormat() };
  msaa_buffer_ = ed_->render_graph->add<TextureResource>(
    "MSAA color buffer", TextureUsage::ColorBuffer, color_format, sample_count);

  auto* depth_buf{ ed_->render_graph->add<TextureResource>(
    "depth buffer", TextureUsage::DepthStencilBuffer,
    ed_->device.findDepthFormat(), sample_count) };

  // TODO: handle resolve buffers implicitly in render graph
  back_buffer_ = ed_->render_graph->add<TextureResource>(
    "back buffer", TextureUsage::BackBuffer, color_format);

  index_buffer_ = ed_->render_graph->add<BufferResource<uint32_t>>(
    "index buffer", BufferUsage::IndexBuffer);
  index_buffer_->uploadData<uint32_t>(indices_);

  vertex_buffer_ = ed_->render_graph->add<BufferResource<GpuVertex>>(
    "vertex buffer", BufferUsage::VertexBuffer);
  vertex_buffer_->addVertexAttribute(VK_FORMAT_R32G32B32_SFLOAT,
                                     offsetof(GpuVertex, pos));
  vertex_buffer_->addVertexAttribute(VK_FORMAT_R32_SFLOAT,
                                     offsetof(GpuVertex, uv_x));
  vertex_buffer_->addVertexAttribute(VK_FORMAT_R32G32B32_SFLOAT,
                                     offsetof(GpuVertex, normal));
  vertex_buffer_->addVertexAttribute(VK_FORMAT_R32_SFLOAT,
                                     offsetof(GpuVertex, uv_y));
  vertex_buffer_->addVertexAttribute(VK_FORMAT_R32G32B32A32_SFLOAT,
                                     offsetof(GpuVertex, color));
  vertex_buffer_->setElementSize(sizeof(GpuVertex));
  vertex_buffer_->uploadData<GpuVertex>(vertices_);

  auto* main_stage = ed_->render_graph->add<GraphicsStage>("main stage");
  // Write order matters here (for now)
  main_stage->writesTo(msaa_buffer_); // clear value index 0 is clear color
  main_stage->writesTo(depth_buf);    // clear value index 1 is depth stencil
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

  for (const auto& shader : ed_->shaders) {
    main_stage->usesShader(shader);
  }

  // TODO: Only one descriptor set is currently allocated for any given
  // ResourceDescriptor. Additionally, for each frame in flight, the
  // descriptor set bound is identical to the previous frame.This means that
  // it is (for now) okay to only add a single layout for each resource
  // descriptor that applies to all frames. for (const auto& descriptor :
  // descriptors_)
  main_stage->addDescriptorLayout(ed_->scene_data_descriptor_layout);
}

void VulkanEngine::recreateSwapchain()
{
  window_.waitForFocus();
  ed_->device.waitIdle();
  ed_->swapchain.setupSwapchain(
    ed_->device, ed_->surface, VkExtent2D{ window_.width(), window_.height() });
  // TODO: experiment with render graph creation/compilation. It is not
  // necessary to rebuild the whole thing on every swapchain invalidation.
  ed_->render_graph.reset();
  ed_->render_graph =
    std::make_unique<RenderGraph>(ed_->device, ed_->swapchain);
  setupRenderGraph();
  // Reset first to destroy ImGui context
  ed_->imgui_overlay.reset();
  ed_->imgui_overlay = std::make_unique<ImGuiOverlay>(
    ed_->device, ed_->swapchain, ed_->render_graph.get(), back_buffer_);
  ed_->render_graph->compile();
}

void VulkanEngine::updateScene(uint32_t current_image)
{
  // TODO: if scene has changed, rebuild vertices/indices
  static size_t last_scene_node_count{ 0 };
  if (loaded_scenes.size() != last_scene_node_count) {
    buildBuffers();
    last_scene_node_count = loaded_scenes.size();
  }

  main_draw_context_.opaque_surfaces.clear();
  loaded_scenes["Suzanne"]->draw(Mat4f{ 1.f }, main_draw_context_);

  static StopWatch stop_watch;
  float            time{ stop_watch.seconds(false) };

  Mat4f model{ glm::rotate(Mat4f(1.0f), time * glm::radians(20.0f),
                           Vec3f(0.0f, 0.0f, 1.0f)) };
  Mat4f view{ glm::lookAt(Vec3f(2.0f, 2.0f, 2.0f), Vec3f(0.0f, 0.0f, 0.0f),
                          Vec3f(0.0f, 0.0f, 1.0f)) };
  Mat4f proj{ glm::perspective(
    glm::radians(45.0f),
    ed_->swapchain.extent().width /
      static_cast<float>(ed_->swapchain.extent().height),
    0.1f, 10.0f) };
  GpuModelData model_data[]{ { .model_mat = model } };
  proj[1][1] *= -1;
  scene_data_.proj               = proj;
  scene_data_.view               = view;
  scene_data_.viewproj           = proj * view;
  scene_data_.ambient_color      = {};
  scene_data_.sunlight_color     = {};
  scene_data_.sunlight_direction = {};
  GpuSceneData scene_data[]{ scene_data_ };
  ed_->frames_in_flight[current_image].scene_data_buffer.uploadData(scene_data);
  ed_->frames_in_flight[current_image].model_data_buffer.uploadData(model_data);
}

void VulkanEngine::drawGeometry(const PhysicalStage&     physical,
                                const wr::CommandBuffer& cb)
{
  const auto& swapchain{ ed_->swapchain };
  const auto& device{ ed_->device };

  const size_t surface_count{ main_draw_context_.opaque_surfaces.size() };
  size_t       idx_offset{ 0 };
  for (size_t i = 0; i < surface_count; ++i) {
    const RenderObject& draw{ main_draw_context_.opaque_surfaces[i] };
    // cb.bindDescriptorSets(descriptors_[frame_index_].descriptorSets(),
    //                       physical.pipelineLayout());
    FrameData&      frame{ ed_->frames_in_flight[frame_index_] };
    VkDescriptorSet frame_descriptor{ frame.descriptors.allocate(
      device, ed_->scene_data_descriptor_layout) };
    VkDescriptorSet viking_descriptor{ frame.descriptors.allocate(
      device, ed_->viking_model_descriptor_layout) };

    DescriptorWriter writer;
    writer.writeUniformBuffer(0, frame.scene_data_buffer, 0)
      .updateSet(device, frame_descriptor);
    writer.reset();

    writer.writeUniformBuffer(0, frame.model_data_buffer, 0)
      .writeCombinedImageSampler(1, ed_->viking_texture,
                                 ed_->default_sampler_linear)
      .updateSet(device, viking_descriptor);

    // cb.bindDescriptorSets(draw.material->descriptor.descriptorSets(),
    // physical.pipelineLayout(), 1);

    cb.bindPipeline(*draw.material->data.pipeline);
    std::vector<VkDescriptorSet> descriptor_sets{
      frame_descriptor, draw.material->data.descriptor_set
    };
    cb.bindDescriptorSets(descriptor_sets,
                          draw.material->data.pipeline->layout());

    const VkViewport viewports[] = { {
      .x        = 0.0f,
      .y        = 0.0f,
      .width    = static_cast<float>(swapchain.extent().width),
      .height   = static_cast<float>(swapchain.extent().height),
      .minDepth = 0.0f,
      .maxDepth = 1.0f,
    } };
    cb.setViewport(viewports, 0);

    const VkRect2D scissors[] = { {
      .offset = { 0, 0 },
      .extent = swapchain.extent(),
    } };
    cb.setScissor(scissors, 0);

    // TODO: push constants

    cb.drawIndexed(draw.index_count, 1, draw.first_index + idx_offset);
    idx_offset += draw.index_count;
  }
}

// std::shared_ptr<Material> VulkanEngine::getDefaultMaterialData() const
// {
//   return std::make_shared<Material>(ed_->default_material_data);
// }

void VulkanEngine::updateImGui(std::function<void()> const& lambda)
{
  ImGuiIO& io    = ImGui::GetIO();
  io.DisplaySize = ImVec2(static_cast<float>(ed_->swapchain.extent().width),
                          static_cast<float>(ed_->swapchain.extent().height));
  io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
  ImGui::NewFrame();
  lambda();
  ImGui::EndFrame();
  ImGui::Render();
  ed_->imgui_overlay->update(ed_->frames_in_flight[frame_index_].descriptors);
}

// TODO: move this to where it is relevant
// void VulkanEngine::uploadMesh(std::span<const Point3f> positions,
//                               std::span<const Vec2f>   texcoords,
//                               std::span<const Color4f> colors,
//                               std::span<const Vec3f>   normals)
// {
//   vertices_.clear();
//   indices_.clear();
//
//   // TODO:
//   // De-duplication should be taken care of earlier, e.g when loading mesh
//   // uploadMesh should look up the mesh's vertex/index buffer resource and
//   do
//   // resource->uploadData<GpuVertex>(vertices) (though this requires a
//   vector of
//   // GpuVertex...)
//   std::unordered_map<GpuVertex, uint32_t> unique_vertices{};
//   // Vertex deduplication
//   for (uint32_t i = 0; i < positions.size(); ++i) {
//     GpuVertex v{ positions[i], texcoords[i], colors[i], normals[i] };
//     if (unique_vertices.count(v) == 0) {
//       unique_vertices[v] = static_cast<uint32_t>(vertices_.size());
//       vertices_.push_back(v);
//     }
//     indices_.push_back(unique_vertices[v]);
//   }
//   log_->debug("Vertex deduplication before and after {} -> {}",
//               positions.size(), vertices_.size());
//   // Recreate swapchain to trigger recompilation of render graph with new
//   // vertex/index buffers. TODO: improve
//   recreateSwapchain();
// }

void VulkanEngine::drawFrame()
{
  const auto& swapchain{ ed_->swapchain };
  const auto& device{ ed_->device };
  if (swapchain_invalidated_) {
    recreateSwapchain();
    swapchain_invalidated_ = false;
    return;
  }

  updateScene(frame_index_); // move

  FrameData& frame{ ed_->frames_in_flight[frame_index_] };

  // Wait until the previous command buffer with the current frame index
  // has finished executing
  if (likely(frame.cmd_buf != nullptr))
    frame.cmd_buf->waitFence();

  frame.descriptors.resetPools();

  const uint32_t image_index{ swapchain.acquireNextImage(
    frame_index_, swapchain_invalidated_) };
  if (swapchain_invalidated_) {
    // Skip rendering, swapchain is recreated on next call
    return;
  }

  const auto& cb{ device.requestCommandBuffer() };
  frame.cmd_buf = &cb;
  ed_->render_graph->render(image_index, cb);

  VkPipelineStageFlags wait_stages[]{
    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
  };

  const VkSubmitInfo submit_info{
    .sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO,
    .pNext                = {},
    .waitSemaphoreCount   = 1,
    .pWaitSemaphores      = swapchain.imageAvailableSemaphore(frame_index_),
    .pWaitDstStageMask    = wait_stages,
    .commandBufferCount   = 1,
    .pCommandBuffers      = cb.ptr(),
    .signalSemaphoreCount = 1,
    .pSignalSemaphores    = swapchain.renderFinishedSemaphore(frame_index_),
  };

  // Submit without waiting (we wait at the beginning of this function)
  cb.submit(submit_info);

  const VkPresentInfoKHR present_info{
    .sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
    .pNext              = {},
    .waitSemaphoreCount = 1,
    .pWaitSemaphores    = swapchain.renderFinishedSemaphore(frame_index_),
    .swapchainCount     = 1,
    .pSwapchains        = swapchain.ptr(),
    .pImageIndices      = &image_index,
    .pResults           = {},
  };

  swapchain.present(present_info, swapchain_invalidated_);

  frame_index_ = (frame_index_ + 1) % max_frames_in_flight;
}

std::string VulkanEngine::deviceName() const { return ed_->device.name(); }

void VulkanEngine::buildMaterialPipelines(GltfMetallicRoughness& material)
{
  const auto& device{ ed_->device };

  const VkPushConstantRange matrix_range{
    .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
    .offset     = 0,
    .size       = sizeof(GpuDrawPushConstants),
  };

  DescriptorSetLayoutBuilder layout_builder;
  layout_builder
    .addUniformBuffer(0,
                      VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
    .addCombinedImageSampler(1, VK_SHADER_STAGE_FRAGMENT_BIT)
    .addCombinedImageSampler(2, VK_SHADER_STAGE_FRAGMENT_BIT);

  material.material_layout = layout_builder.build(device, 0);

  wr::Shader vert_shader{ device, "material vertex shader", "mesh.vert.spv",
                          VK_SHADER_STAGE_VERTEX_BIT };
  wr::Shader frag_shader{ device, "material fragment shader", "mesh.frag.spv",
                          VK_SHADER_STAGE_FRAGMENT_BIT };
  PipelineBuilder pipeline_builder;
  pipeline_builder.addDescriptorSetLayout(ed_->scene_data_descriptor_layout)
    .addDescriptorSetLayout(material.material_layout)
    .addPushConstantRange(matrix_range)
    .setShaders(vert_shader, frag_shader)
    .setInputTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
    .setPolygonMode(VK_POLYGON_MODE_FILL)
    .setCullMode(VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE)
    .setMultisamplingNone()
    .disableBlending()
    .enableDepthtest(
      true, VK_COMPARE_OP_GREATER_OR_EQUAL) // TODO: not the same compare op
                                            // as i used in rendergraph

    // render format
    //.setColorAttachmentFormat(back_buffer_->format_)
    //.setDepthFormat(depth_buffer->format_);

    // TODO: THE FORMATS SHOULD BE SET FROM TEXTURE RESOURCE
    .setColorAttachmentFormat(ed_->swapchain.imageFormat())
    .setDepthFormat(device.findDepthFormat());

  // finally build the pipeline
  // TODO: pipeline names should ultimately be constructed from the material
  // information
  material.opaque_pipeline =
    pipeline_builder.build(device, "GltfMetallicRoughness opaque pipeline");

  // create the transparent variant
  pipeline_builder.enableBlendingAdditive().enableDepthtest(
    false, VK_COMPARE_OP_GREATER_OR_EQUAL);

  material.transparent_pipeline = pipeline_builder.build(
    device, "GltfMetallicRoughness transparent pipeline");
}

} // namespace eldr::vk
namespace std {
size_t
hash<eldr::vk::GpuVertex>::operator()(eldr::vk::GpuVertex const& vertex) const
{
  size_t value{ hash<Point3f>()(vertex.pos) };
  value = eldr::hashCombine(value, hash<float>()(vertex.uv_x));
  value = eldr::hashCombine(value, hash<Vec3f>()(vertex.normal));
  value = eldr::hashCombine(value, hash<float>()(vertex.uv_y));
  value = eldr::hashCombine(value, hash<Color4f>()(vertex.color));
  return value;
};
} // namespace std
