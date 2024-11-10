// Ensure that vma implementation is included
#define VMA_IMPLEMENTATION
#include <eldr/app/window.hpp>
#include <eldr/core/fwd.hpp>
#include <eldr/core/logger.hpp>
#include <eldr/core/math.hpp>
#include <eldr/core/platform.hpp>
#include <eldr/vulkan/common.hpp>
#include <eldr/vulkan/engine.hpp>
#include <eldr/vulkan/rendergraph.hpp>
#include <eldr/vulkan/vertex.hpp>
#include <eldr/vulkan/wrappers/buffer.hpp>
#include <eldr/vulkan/wrappers/commandbuffer.hpp>
#include <eldr/vulkan/wrappers/debugutilsmessenger.hpp>
#include <eldr/vulkan/wrappers/descriptor.hpp>
#include <eldr/vulkan/wrappers/descriptorbuilder.hpp>
#include <eldr/vulkan/wrappers/device.hpp>
#include <eldr/vulkan/wrappers/gputexture.hpp>
#include <eldr/vulkan/wrappers/instance.hpp>
#include <eldr/vulkan/wrappers/shader.hpp>
#include <eldr/vulkan/wrappers/surface.hpp>
#include <eldr/vulkan/wrappers/swapchain.hpp>
#include <engine_config.hpp>

#include <GLFW/glfw3.h>
#include <backends/imgui_impl_glfw.h>
#include <imgui.h>

#include <memory>
#include <string>

namespace eldr::vk {
struct UniformBufferObject {
  ELDR_IMPORT_CORE_TYPES();
  alignas(16) Mat4f model;
  alignas(16) Mat4f view;
  alignas(16) Mat4f proj;
};

VulkanEngine::VulkanEngine(const Window& window)
  : window_(window), in_flight_cmd_bufs_(std::vector<const wr::CommandBuffer*>(
                       max_frames_in_flight, nullptr))
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
  device_ =
    std::make_unique<wr::Device>(*instance_, *surface_, device_extensions);

  // ---------------------------------------------------------------------------
  // Set MSAA sample count
  // TODO: maybe this should be configurable rather than using max possible
  // samples
  // ---------------------------------------------------------------------------
  msaa_sample_count_ = device_->maxMsaaSampleCount();
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
  // Create uniform buffers and descriptors
  // ---------------------------------------------------------------------------
  createUniformBuffers();
  createResourceDescriptors();
  //  ---------------------------------------------------------------------------
  //  Create render graph
  //  ---------------------------------------------------------------------------
  recreateSwapchain();
}

VulkanEngine::~VulkanEngine() { vkDeviceWaitIdle(device_->logical()); }

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

void VulkanEngine::createUniformBuffers()
{
  for (uint8_t i = 0; i < max_frames_in_flight; ++i) {
    uniform_buffers_.emplace_back(*device_, sizeof(UniformBufferObject),
                                  VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                  VMA_MEMORY_USAGE_GPU_ONLY, "Uniform buffer");
  }
}

void VulkanEngine::createResourceDescriptors()
{
  for (size_t i = 0; i < max_frames_in_flight; ++i) {
    wr::DescriptorBuilder descriptor_builder(*device_);
    descriptors_.emplace_back(
      descriptor_builder
        .addUniformBuffer<UniformBufferObject>(uniform_buffers_[i].get(), 0)
        .addCombinedImageSampler(textures_[0].sampler(),
                                 textures_[0].imageView(), 1)
        .build(textures_[0].name())); // TODO: textures created with bitmaps are
                                      // named "undefined" by default. Get name
                                      // some other way.
  }

  // ===========================================================================
  // VkDescriptorSetLayoutBinding ubo_layout_binding{};
  // ubo_layout_binding.binding            = 0;
  // ubo_layout_binding.descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  // ubo_layout_binding.descriptorCount    = 1;
  // ubo_layout_binding.stageFlags         = VK_SHADER_STAGE_VERTEX_BIT;
  // ubo_layout_binding.pImmutableSamplers = nullptr; // optional

  // VkDescriptorSetLayoutBinding sampler_layout_binding{};
  // sampler_layout_binding.binding         = 1;
  // sampler_layout_binding.descriptorCount = 1;
  // sampler_layout_binding.descriptorType =
  //   VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  // sampler_layout_binding.pImmutableSamplers = nullptr;
  // sampler_layout_binding.stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT;

  // const std::vector<VkDescriptorSetLayoutBinding> bindings{
  //   ubo_layout_binding, sampler_layout_binding
  // };

  // for (size_t i = 0; i < max_frames_in_flight; ++i) {
  //   VkDescriptorBufferInfo buffer_info{};
  //   buffer_info.buffer = uniform_buffers_[i].get();
  //   buffer_info.offset = 0;
  //   buffer_info.range  = sizeof(UniformBufferObject);

  //   VkDescriptorImageInfo image_info{};
  //   image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  //   image_info.imageView   = texture_sampler_.texture.view().get();
  //   image_info.sampler     = texture_sampler_.sampler.get();

  //   std::vector<VkWriteDescriptorSet> descriptor_writes{};
  //   descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  //   descriptor_writes[0].dstArrayElement = 0;
  //   descriptor_writes[0].descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  //   descriptor_writes[0].descriptorCount = 1;
  //   // Either pBufferInfo or pImageInfo or pTexelBufferView is set
  //   descriptor_writes[0].pBufferInfo      = &buffer_info;
  //   descriptor_writes[0].pImageInfo       = nullptr;
  //   descriptor_writes[0].pTexelBufferView = nullptr;

  //   descriptor_writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  //   descriptor_writes[1].dstArrayElement = 0;
  //   descriptor_writes[1].descriptorType =
  //     VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  //   descriptor_writes[1].descriptorCount = 1;
  //   // Either pBufferInfo or pImageInfo or pTexelBufferView is set
  //   descriptor_writes[1].pBufferInfo      = nullptr;
  //   descriptor_writes[1].pImageInfo       = &image_info;
  //   descriptor_writes[1].pTexelBufferView = nullptr;

  //   descriptors_.emplace_back(*device_, bindings, descriptor_writes);
  // }
}

void VulkanEngine::setupRenderGraph()
{
  // 1 sample for back buffer, max possible sample count for color images and
  // depth buffer. TODO: may want to be more flexible when setting sample count.
  const VkSampleCountFlagBits sample_count = device_->maxMsaaSampleCount();

  back_buffer_ = render_graph_->add<TextureResource>(
    "back buffer", TextureUsage::back_buffer, swapchain_->imageFormat());

  auto* msaa_buffer = render_graph_->add<TextureResource>(
    "MSAA color buffer", TextureUsage::offscreen_buffer,
    swapchain_->imageFormat(), sample_count);

  auto* depth_buffer = render_graph_->add<TextureResource>(
    "depth buffer", TextureUsage::depth_stencil_buffer,
    device_->findDepthFormat(), sample_count);

  index_buffer_ = render_graph_->add<BufferResource>("index buffer",
                                                     BufferUsage::index_buffer);
  index_buffer_->uploadData<uint32_t>(indices_);

  vertex_buffer_ = render_graph_->add<BufferResource>(
    "vertex buffer", BufferUsage::vertex_buffer);
  vertex_buffer_->addVertexAttribute(VK_FORMAT_R32G32B32_SFLOAT,
                                     offsetof(GpuVertex, pos));
  vertex_buffer_->addVertexAttribute(VK_FORMAT_R32G32B32_SFLOAT,
                                     offsetof(GpuVertex, color));
  vertex_buffer_->addVertexAttribute(VK_FORMAT_R32G32_SFLOAT,
                                     offsetof(GpuVertex, uv));
  vertex_buffer_->uploadData<GpuVertex>(vertices_);

  auto* main_stage = render_graph_->add<GraphicsStage>("main stage");
  main_stage->writesTo(back_buffer_);
  main_stage->writesTo(msaa_buffer);
  main_stage->writesTo(depth_buffer);
  main_stage->readsFrom(index_buffer_);
  main_stage->readsFrom(vertex_buffer_);
  main_stage->bindBuffer(vertex_buffer_, 0);
  main_stage->setClearsScreen(true);
  main_stage->setDepthOptions(true, true);
  main_stage->setOnRecord(
    [&](const PhysicalStage& physical, const wr::CommandBuffer& cb) {
      cb.bindDescriptorSets(descriptors_[0].descriptorSets(),
                            physical.pipelineLayout());
      cb.drawIndexed(static_cast<std::uint32_t>(indices_.size()));
    });

  for (const auto& shader : shaders_) {
    main_stage->usesShader(shader);
  }

  // TODO: blend attachment of main stage?
  main_stage->addDescriptorLayout(descriptors_[0].descriptorSetLayout());
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

void VulkanEngine::updateUniformBuffer(uint32_t current_image)
{
  static auto start_time   = std::chrono::high_resolution_clock::now();
  auto        current_time = std::chrono::high_resolution_clock::now();
  float       time = std::chrono::duration<float, std::chrono::seconds::period>(
                 current_time - start_time)
                 .count();

  UniformBufferObject ubo{};
  ubo.model = glm::rotate(Mat4f(1.0f), time * glm::radians(90.0f),
                          Vec3f(0.0f, 0.0f, 1.0f));
  ubo.view  = glm::lookAt(Vec3f(2.0f, 2.0f, 2.0f), Vec3f(0.0f, 0.0f, 0.0f),
                          Vec3f(0.0f, 0.0f, 1.0f));
  ubo.proj  = glm::perspective(glm::radians(45.0f),
                               swapchain_->extent().width /
                                 (float) swapchain_->extent().height,
                               0.1f, 10.0f);
  ubo.proj[1][1] *= -1;
  memcpy(uniform_buffers_mapped_[current_image], &ubo, sizeof(ubo));
}

// TODO: move this to where it is relevant
void VulkanEngine::submitGeometry(const std::vector<Vec3f>& positions,
                                  const std::vector<Vec2f>& texcoords)
{
  vertices_.clear();
  indices_.clear();
  std::unordered_map<GpuVertex, uint32_t> unique_vertices{};
  // Vertex deduplication
  for (uint32_t i = 0; i < positions.size(); ++i) {
    GpuVertex v{ positions[i], { 1.0f, 1.0f, 1.0f }, texcoords[i] };
    if (unique_vertices.count(v) == 0) {
      unique_vertices[v] = static_cast<uint32_t>(vertices_.size());
      vertices_.push_back(v);
    }
    indices_.push_back(unique_vertices[v]);
  }
  // Recreate swapchain to trigger recompilation of render graph with new
  // vertex/index buffers. TODO: improve
  recreateSwapchain();
}

void VulkanEngine::drawFrame()
{
  if (framebuffer_resized_) {
    recreateSwapchain();
    framebuffer_resized_ = false;
    return;
  }

  // Wait until the previous command buffer with the current frame index
  // has finished executing
  if (likely(in_flight_cmd_bufs_[current_frame_] != nullptr))
    in_flight_cmd_bufs_[current_frame_]->waitFence();

  const uint32_t image_index = swapchain_->acquireNextImage(current_frame_);

  const auto& cb                      = device_->requestCommandBuffer();
  in_flight_cmd_bufs_[current_frame_] = &cb;
  render_graph_->render(image_index, cb);

  updateUniformBuffer(current_frame_);

  VkPipelineStageFlags wait_stages[] = {
    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
  };

  const VkSubmitInfo submit_info{
    .sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO,
    .pNext                = {},
    .waitSemaphoreCount   = 1,
    .pWaitSemaphores      = swapchain_->imageAvailableSemaphore(current_frame_),
    .pWaitDstStageMask    = wait_stages,
    .commandBufferCount   = 1,
    .pCommandBuffers      = &cb.get(),
    .signalSemaphoreCount = 1,
    .pSignalSemaphores    = swapchain_->renderFinishedSemaphore(current_frame_),
  };

  // Submit without waiting (we wait at the beginning of this function)
  cb.submit(submit_info);

  VkSwapchainKHR         swapchains[] = { swapchain_->get() };
  const VkPresentInfoKHR present_info{
    .sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
    .pNext              = {},
    .waitSemaphoreCount = 1,
    .pWaitSemaphores    = swapchain_->renderFinishedSemaphore(current_frame_),
    .swapchainCount     = 1,
    .pSwapchains        = swapchains,
    .pImageIndices      = &image_index,
    .pResults           = {},
  };

  swapchain_->present(present_info);

  current_frame_ = (current_frame_ + 1) % max_frames_in_flight;
}

} // namespace eldr::vk
