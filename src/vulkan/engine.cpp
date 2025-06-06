// Ensure that vma implementation is included
#include <vulkan/vulkan_core.h>
#define VMA_IMPLEMENTATION
#include <eldr/app/window.hpp>
#include <eldr/buildinfo.hpp>
#include <eldr/core/bitmap.hpp>
#include <eldr/core/hash.hpp>
#include <eldr/core/logger.hpp>
#include <eldr/core/math.hpp>
#include <eldr/core/platform.hpp>
#include <eldr/core/stopwatch.hpp>
#include <eldr/render/mesh.hpp>
#include <eldr/render/scene.hpp>
#include <eldr/vulkan/descriptorallocator.hpp>
#include <eldr/vulkan/descriptorsetlayoutbuilder.hpp>
#include <eldr/vulkan/descriptorwriter.hpp>
#include <eldr/vulkan/engine.hpp>
#include <eldr/vulkan/imgui.hpp>
#include <eldr/vulkan/material.hpp>
#include <eldr/vulkan/pipelinebuilder.hpp>
#include <eldr/vulkan/rendergraph.hpp>
#include <eldr/vulkan/vktypes.hpp>
#include <eldr/vulkan/vulkan.hpp>
#include <eldr/vulkan/wrappers/buffer.hpp>
#include <eldr/vulkan/wrappers/commandbuffer.hpp>
#include <eldr/vulkan/wrappers/debugutilsmessenger.hpp>
#include <eldr/vulkan/wrappers/device.hpp>
#include <eldr/vulkan/wrappers/instance.hpp>
#include <eldr/vulkan/wrappers/sampler.hpp>
#include <eldr/vulkan/wrappers/shader.hpp>
#include <eldr/vulkan/wrappers/surface.hpp>
#include <eldr/vulkan/wrappers/swapchain.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <imgui.h>

#include <memory>
#include <string>

using namespace eldr::core;
using namespace eldr::vk::wr;

namespace eldr::vk {
// -----------------------------------------------------------------------------
// Engine types
// -----------------------------------------------------------------------------
struct FrameData {
  DescriptorAllocator      descriptors;
  Buffer<GpuSceneData>     scene_data_buffer;
  Buffer<GpuModelData>     model_data_buffer;
  const wr::CommandBuffer* cmd_buf;
};

// TODO: is this even used
struct GpuMeshBuffers {
  VkDeviceAddress vertex_buffer_address;
};

struct VulkanEngine::Settings {
  VkSampleCountFlagBits msaa_sample_count;
};

struct VulkanEngine::EngineData {
  Instance            instance;
  DebugUtilsMessenger debug_messenger;
  Surface             surface;
  Device              device;
  Swapchain           swapchain;
  DescriptorAllocator global_descriptor_allocator;

  Buffer<GpuVertex> vertex_buffer;
  Buffer<uint32_t>  index_buffer;
  // std::vector<GpuVertex> vertices;
  // std::vector<uint32_t>  indices;

  std::unique_ptr<RenderGraph>  render_graph;
  std::unique_ptr<ImGuiOverlay> imgui_overlay;
  std::vector<Image>            textures;
  std::vector<Shader>           shaders; // shader module is not needed after
                               // building pipeline so check if this can be
                               // rearranged
  std::vector<FrameData> frames_in_flight;

  // The data below is experimental, default data
  DescriptorSetLayout scene_data_descriptor_layout;
  DescriptorSetLayout model_data_descriptor_layout;
  // DescriptorSetLayout   viking_model_descriptor_layout;
  GltfMetallicRoughness metal_rough_material;
  MaterialInstance      default_material_data;

  Image white_texture;
  Image error_texture;

  Image   viking_texture;
  Sampler default_sampler_linear;
};

// -----------------------------------------------------------------------------
// Engine
// -----------------------------------------------------------------------------
VulkanEngine::VulkanEngine(const app::Window& window)
  : window_(window), d_(std::make_unique<EngineData>()),
    s_(std::make_unique<Settings>())
{
  // ---------------------------------------------------------------------------
  // Create instance
  // ---------------------------------------------------------------------------
  const VkApplicationInfo app_info{
    .sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
    .pNext              = {},
    .pApplicationName   = EL_NAME,
    .applicationVersion = VK_MAKE_API_VERSION(
      EL_VER_VARIANT, EL_VER_MAJOR, EL_VER_MINOR, EL_VER_PATCH),
    .pEngineName   = EL_ENGINE_NAME,
    .engineVersion = VK_MAKE_API_VERSION(EL_ENGINE_VER_VARIANT,
                                         EL_ENGINE_VER_MAJOR,
                                         EL_ENGINE_VER_MINOR,
                                         EL_ENGINE_VER_PATCH),
    .apiVersion    = VK_API_VERSION_1_3,
  };

  d_->instance = Instance{ app_info, window_.instanceExtensions() };
  // ---------------------------------------------------------------------------
  // Create debug messenger
  // ---------------------------------------------------------------------------
#ifdef ELDR_VULKAN_DEBUG_REPORT
  d_->debug_messenger = DebugUtilsMessenger{ d_->instance };
#endif
  // ---------------------------------------------------------------------------
  // Create surface
  // ---------------------------------------------------------------------------
  d_->surface = Surface{ d_->instance, window_ };
  // ---------------------------------------------------------------------------
  // Create device
  // ---------------------------------------------------------------------------
  std::vector<const char*> device_extensions; // required
  device_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
  device_extensions.push_back(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
  device_extensions.push_back(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
#ifdef VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME
  device_extensions.push_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
#endif
  // Pass log_ pointer to device so that all wrapper objects that have a
  // reference to the device can access the same logger
  d_->device = Device{ d_->instance, d_->surface, device_extensions };

  // ---------------------------------------------------------------------------
  // Create swapchain
  // ---------------------------------------------------------------------------
  d_->swapchain = Swapchain(
    d_->device, d_->surface, VkExtent2D{ window_.width(), window_.height() });
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
  //  Settings (Init defaults, some of these should be possible to change later
  //  on using the GUI
  //  ---------------------------------------------------------------------------
  s_->msaa_sample_count = d_->device.findMaxMsaaSampleCount();

  // ---------------------------------------------------------------------------
  // Init default data
  // ---------------------------------------------------------------------------
  initDefaultData();
  buildMaterialPipelines(d_->metal_rough_material);

  //  ---------------------------------------------------------------------------
  //  Create render graph
  //  ---------------------------------------------------------------------------
  recreateSwapchain();
}

VulkanEngine::~VulkanEngine() { d_->device.waitIdle(); }

// TODO: This is cursed and needs to be refactored
GltfMetallicRoughness& VulkanEngine::metalRoughMaterial() const
{
  return d_->metal_rough_material;
}
const Image&   VulkanEngine::whiteImage() const { return d_->white_texture; }
const Image&   VulkanEngine::errorImage() const { return d_->error_texture; }
const Sampler& VulkanEngine::defaultSamplerLinear() const
{
  return d_->default_sampler_linear;
}
// -----------------------------------------------------

void VulkanEngine::loadTextures()
{
  Log(Trace, "Loading textures...");
  const char* env_p = std::getenv("ELDR_DIR");
  if (env_p == nullptr) {
    Throw("Environment not set up correctly");
  }
  // TODO: build a vector of texture filenames from some configuration and load
  // each texture
  const std::string     texture_path = "/assets/textures/viking_room.png";
  std::filesystem::path filepath{ std::string(env_p) + texture_path };

  Bitmap bitmap{ filepath };
  if (bitmap.pixelFormat() != Bitmap::PixelFormat::RGBA)
    bitmap.rgbToRgba();
  d_->textures.emplace_back(d_->device, bitmap);
}

void VulkanEngine::loadShaders()
{
  // TODO: get a list of shaders to load from config or something and load all
  // of them into shaders vector
  d_->shaders.emplace_back(d_->device,
                           "default vertex shader",
                           "main.vert.spv",
                           VK_SHADER_STAGE_VERTEX_BIT);
  d_->shaders.emplace_back(d_->device,
                           "default frag shader",
                           "main.frag.spv",
                           VK_SHADER_STAGE_FRAGMENT_BIT);
}

void VulkanEngine::setupFrameData()
{
  for (uint8_t i = 0; i < max_frames_in_flight; ++i) {

    // TODO: I will need to come back to this eventually and understand how to
    // set this dynamically, I can't thik atm
    std::vector<PoolSizeRatio> frame_sizes{
      { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3 },
      { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3 }
    };

    constexpr size_t elem_count{ 1 };
    d_->frames_in_flight.push_back({
      .descriptors       = DescriptorAllocator{ 1000, frame_sizes },
      .scene_data_buffer = { d_->device,
                             "Scene data uniform buffer",
                             elem_count,
                             VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                             VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT },
      .model_data_buffer = { d_->device,
                             "Model data uniform buffer",
                             elem_count,
                             VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                             VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT },
      .cmd_buf           = nullptr, // Set later when drawing frames
    });
  }
}

void VulkanEngine::initDescriptors()
{

  DescriptorSetLayoutBuilder layout_builder;
  layout_builder.addUniformBuffer(
    0, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
  d_->scene_data_descriptor_layout = layout_builder.build(d_->device, 0);

  layout_builder.reset();

  layout_builder.addUniformBuffer(0, VK_SHADER_STAGE_VERTEX_BIT);
  d_->model_data_descriptor_layout = layout_builder.build(d_->device, 0);

  // layout_builder.addUniformBuffer(0, VK_SHADER_STAGE_VERTEX_BIT)
  //   .addCombinedImageSampler(1, VK_SHADER_STAGE_FRAGMENT_BIT);
  // d_->viking_model_descriptor_layout = layout_builder.build(d_->device, 0);
}

void VulkanEngine::initDefaultData()
{
  const Device& device{ d_->device };
  // Create default white texture
  d_->white_texture = Image{ device, Bitmap::createDefaultWhite() };

  // Create default checkerboard error texture
  d_->error_texture = Image{ device, Bitmap::createCheckerboard() };

  // Create default linear sampler
  d_->default_sampler_linear = Sampler{ device,
                                        VK_FILTER_LINEAR,
                                        VK_FILTER_LINEAR,
                                        VK_SAMPLER_MIPMAP_MODE_LINEAR,
                                        d_->white_texture.mipLevels() };
}

void VulkanEngine::updateBuffers()
{
  std::vector<GpuVertex>                  vertices;
  std::vector<uint32_t>                   indices;
  std::unordered_map<GpuVertex, uint32_t> unique_vertices{};
  // Vertex deduplication
  size_t total_vtx_count{ 0 };
  for (const auto& es : loaded_scenes_) {
    for (const auto& en : es.second->nodes) {
      if (const auto& mesh_node{
            dynamic_cast<const MeshNode*>(en.second.get()) }) {
        const auto&  mesh{ mesh_node->mesh };
        const size_t vtx_count{ mesh->vtxPositions().size() };
        total_vtx_count += vtx_count;
        for (uint32_t i = 0; i < vtx_count; ++i) {
          const float     uv_x{ mesh->vtxTexCoords()[i].x };
          const float     uv_y{ mesh->vtxTexCoords()[i].y };
          const GpuVertex v{ mesh->vtxPositions()[i],
                             uv_x,
                             mesh->vtxNormals()[i],
                             uv_y,
                             mesh->vtxColors()[i] };
          if (unique_vertices.count(v) == 0) {
            unique_vertices[v] = static_cast<uint32_t>(vertices.size());
            vertices.push_back(v);
          }
          indices.push_back(unique_vertices[v]);
        }
      }
    }
  }
  Log(Debug,
      "Vertex deduplication before and after {} -> {}",
      total_vtx_count,
      vertices.size());

  d_->index_buffer = {
    d_->device,
    "index buffer",
    indices,
    VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
  };

  d_->vertex_buffer = {
    d_->device,
    "vertex buffer",
    vertices,
    VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT |
      VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
  };
}

void VulkanEngine::setupRenderGraph()
{
  auto*    graph{ d_->render_graph.get() };
  VkFormat color_format{ d_->swapchain.imageFormat() };

  auto* color_buffer = graph->add<TextureResource>(
    "Color buffer", TextureUsage::Color, color_format);
  color_buffer->setSampleCount(s_->msaa_sample_count);
  color_buffer->resolvesTo(graph->backBuffer());

  auto* depth_buffer{ graph->add<TextureResource>(
    "Depth buffer", TextureUsage::DepthStencil, d_->device.findDepthFormat()) };
  depth_buffer->setSampleCount(s_->msaa_sample_count);

  auto* main_stage = graph->add<GraphicsStage>("Main stage");
  main_stage->writesTo(color_buffer, VK_ATTACHMENT_LOAD_OP_CLEAR)
    .writesTo(depth_buffer, VK_ATTACHMENT_LOAD_OP_CLEAR)
    .setOnRecord([&](const CommandBuffer& cb) { drawGeometry(cb); });
}

void VulkanEngine::recreateSwapchain()
{
  const auto& device{ d_->device };
  auto&       swapchain{ d_->swapchain };
  auto&       graph{ d_->render_graph };
  auto&       overlay{ d_->imgui_overlay };

  window_.waitForFocus();
  device.waitIdle();
  swapchain.setupSwapchain(
    device, d_->surface, { window_.width(), window_.height() });
  // TODO: experiment with render graph creation/compilation. It is not
  // necessary to rebuild the whole thing on every swapchain invalidation.
  graph.reset();
  graph = std::make_unique<RenderGraph>(device, swapchain);
  setupRenderGraph();
  // Reset first to destroy ImGui context
  overlay.reset();
  overlay = std::make_unique<ImGuiOverlay>(device, swapchain, graph.get());
  graph->compile();
}

void VulkanEngine::updateScenes(uint32_t current_image)
{
  static size_t last_scene_node_count{ 0 };
  for (auto& kv : loaded_scenes_) {
    auto& scene = kv.second;
    if (scene->meshes.size() != last_scene_node_count) {
      updateBuffers();
      last_scene_node_count = scene->meshes.size();
    }

    main_draw_context_.opaque_surfaces.clear();
    scene->draw(Mat4f{ 1.f }, main_draw_context_);

    static StopWatch stop_watch;
    float            time{ stop_watch.seconds<float>(false) };

    Mat4f model{ glm::rotate(
      Mat4f(1.0f), time * glm::radians(20.0f), Vec3f(0.0f, 0.0f, 1.0f)) };
    Mat4f view{ glm::lookAt(Vec3f(2.0f, 2.0f, 2.0f),
                            Vec3f(0.0f, 0.0f, 0.0f),
                            Vec3f(0.0f, 0.0f, 1.0f)) };
    Mat4f proj{ glm::perspective(
      glm::radians(45.0f),
      d_->swapchain.extent().width /
        static_cast<float>(d_->swapchain.extent().height),
      0.1f,
      10.0f) };
    proj[1][1] *= -1;
    scene_data_.proj               = proj;
    scene_data_.view               = view;
    scene_data_.viewproj           = proj * view;
    scene_data_.ambient_color      = Vec4f{ .05f };
    scene_data_.sunlight_color     = Vec4f{ 1, 1, 1, 1.f };
    scene_data_.sunlight_direction = Vec4f{ 0, 1, 0.5, 1.f };
    const GpuModelData model_data[]{ { .model_mat = model } };
    const GpuSceneData scene_data[]{ scene_data_ };
    d_->frames_in_flight[current_image].scene_data_buffer.uploadData(
      scene_data);
    d_->frames_in_flight[current_image].model_data_buffer.uploadData(
      model_data);
  }
}

void VulkanEngine::drawGeometry(const CommandBuffer& cb)
{
  const auto& swapchain{ d_->swapchain };
  const auto& device{ d_->device };

  cb.bindIndexBuffer(d_->index_buffer);
  // Using vertex pulling instead, so binding vertex buffer not necessary
  // VkBuffer vbuffers[]{ d_->vertex_buffer.vk() };
  // cb.bindVertexBuffers(vbuffers);

  const size_t    surface_count{ main_draw_context_.opaque_surfaces.size() };
  FrameData&      frame{ d_->frames_in_flight[frame_index_] };
  VkDescriptorSet scene_descriptor{ frame.descriptors.allocate(
    device, d_->scene_data_descriptor_layout) };

  VkDescriptorSet model_descriptor{ frame.descriptors.allocate(
    device, d_->model_data_descriptor_layout) };

  size_t idx_offset{ 0 };

  DescriptorWriter writer;
  writer.writeUniformBuffer(0, frame.scene_data_buffer, 0)
    .updateSet(device, scene_descriptor);
  writer.reset();

  // TODO: model should be incorporated with mesh or draw or something
  writer.writeUniformBuffer(0, frame.model_data_buffer, 0)
    .updateSet(device, model_descriptor);

  for (size_t i{ 0 }; i < surface_count; ++i) {
    const RenderObject& draw{ main_draw_context_.opaque_surfaces[i] };
    // cb.bindDescriptorSets(descriptors_[frame_index_].descriptorSets(),
    //                       physical.pipelineLayout());
    // VkDescriptorSet viking_descriptor{ frame.descriptors.allocate(
    //   device, d_->viking_model_descriptor_layout) };

    //

    // cb.bindDescriptorSets(draw.material->descriptor.descriptorSets(),
    // physical.pipelineLayout(), 1);

    cb.bindPipeline(*draw.material->data.pipeline);

    GpuDrawPushConstants push_constants{
      .world_matrix  = Mat4f{ 1.0f },
      .vertex_buffer = d_->vertex_buffer.getDeviceAddress(),
    };
    cb.pushConstant(draw.material->data.pipeline->layout(),
                    push_constants,
                    VK_SHADER_STAGE_VERTEX_BIT);

    std::vector<VkDescriptorSet> descriptor_sets{
      scene_descriptor, draw.material->data.descriptor_set, model_descriptor
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

    cb.drawIndexed(draw.index_count, 1, draw.first_index + idx_offset);
    idx_offset += draw.index_count;
  }
}

// std::shared_ptr<Material> VulkanEngine::getDefaultMaterialData() const
// {
//   return std::make_shared<Material>(d_->default_material_data);
// }

void VulkanEngine::updateImGui(std::function<void()> const& lambda)
{
  ImGuiIO& io    = ImGui::GetIO();
  io.DisplaySize = ImVec2(static_cast<float>(d_->swapchain.extent().width),
                          static_cast<float>(d_->swapchain.extent().height));
  io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
  ImGui::NewFrame();
  lambda();
  ImGui::EndFrame();
  ImGui::Render();
  d_->imgui_overlay->update(d_->frames_in_flight[frame_index_].descriptors);
}

// TODO: move this to where it is relevant
// void VulkanEngine::uploadMesh(std::span<const Point3f> positions,
//                               std::span<const Vec2f>   texcoords,
//                               std::span<const Color4f> colors,
//                               std::span<const Vec3f>   normals)
// {
//   vertices.clear();
//   indices.clear();
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
//       unique_vertices[v] = static_cast<uint32_t>(vertices.size());
//       vertices.push_back(v);
//     }
//     indices.push_back(unique_vertices[v]);
//   }
//   log_->debug("Vertex deduplication before and after {} -> {}",
//               positions.size(), vertices.size());
//   // Recreate swapchain to trigger recompilation of render graph with new
//   // vertex/index buffers. TODO: improve
//   recreateSwapchain();
// }

void VulkanEngine::drawFrame()
{
  auto&       swapchain{ d_->swapchain };
  const auto& device{ d_->device };
  if (swapchain_invalidated_) {
    recreateSwapchain();
    swapchain_invalidated_ = false;
    return;
  }

  updateScenes(frame_index_); // move

  FrameData& frame{ d_->frames_in_flight[frame_index_] };

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

  const auto& cb = device.requestCommandBuffer();
  frame.cmd_buf  = &cb;

  cb.transitionImageLayout(swapchain.image(image_index),
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
  d_->render_graph->render(cb, swapchain.image(image_index));
  cb.transitionImageLayout(swapchain.image(image_index),
                           VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

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
    .pCommandBuffers      = cb.vkp(),
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
    .pSwapchains        = swapchain.vkp(),
    .pImageIndices      = &image_index,
    .pResults           = {},
  };

  swapchain.present(present_info, swapchain_invalidated_);

  frame_index_ = (frame_index_ + 1) % max_frames_in_flight;
}

std::string VulkanEngine::deviceName() const { return d_->device.name(); }

const Device& VulkanEngine::device() const { return d_->device; }

void VulkanEngine::buildMaterialPipelines(GltfMetallicRoughness& material)
{
  const auto&               device{ d_->device };
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

  Shader          vert_shader{ device,
                      "material vertex shader",
                      "mesh.vert.spv",
                      VK_SHADER_STAGE_VERTEX_BIT };
  Shader          frag_shader{ device,
                      "material fragment shader",
                      "mesh.frag.spv",
                      VK_SHADER_STAGE_FRAGMENT_BIT };
  PipelineBuilder pipeline_builder;
  pipeline_builder.addDescriptorSetLayout(d_->scene_data_descriptor_layout)
    .addDescriptorSetLayout(material.material_layout)
    .addDescriptorSetLayout(d_->model_data_descriptor_layout)
    .addPushConstantRange(matrix_range)
    .setShaders(vert_shader, frag_shader)
    .setInputTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
    .setPolygonMode(VK_POLYGON_MODE_FILL)
    .setCullMode(VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE)
    .setMultisampling(s_->msaa_sample_count)
    .disableBlending()
    .enableDepthtest(true, VK_COMPARE_OP_LESS)

    // render format
    //.setColorAttachmentFormat(back_buffer_->format_)
    //.setDepthFormat(depth_buffer->format_);

    // TODO: THE FORMATS SHOULD BE SET FROM TEXTURE RESOURCE
    .setColorAttachmentFormat(d_->swapchain.imageFormat())
    .setDepthFormat(device.findDepthFormat());

  // finally build the pipeline
  // TODO: pipeline names should ultimately be constructed from the material
  // information
  material.opaque_pipeline =
    pipeline_builder.build(device, "GltfMetallicRoughness opaque");

  // create the transparent variant
  pipeline_builder.enableBlendingAdditive().enableDepthtest(
    false, VK_COMPARE_OP_GREATER_OR_EQUAL);

  material.transparent_pipeline =
    pipeline_builder.build(device, "GltfMetallicRoughness transparent");
}

} // namespace eldr::vk

size_t std::hash<eldr::vk::GpuVertex>::operator()(
  eldr::vk::GpuVertex const& vertex) const
{
  size_t value{ hash<Point3f>()(vertex.pos) };
  value = eldr::hashCombine(value, hash<float>()(vertex.uv_x));
  value = eldr::hashCombine(value, hash<Vec3f>()(vertex.normal));
  value = eldr::hashCombine(value, hash<float>()(vertex.uv_y));
  value = eldr::hashCombine(value, hash<Color4f>()(vertex.color));
  return value;
};
