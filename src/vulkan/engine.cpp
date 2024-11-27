// Ensure that vma implementation is included
#define VMA_IMPLEMENTATION
#include <eldr/app/window.hpp>
#include <eldr/core/exceptions.hpp>
#include <eldr/core/math.hpp>
#include <eldr/core/platform.hpp>
#include <eldr/core/stopwatch.hpp>
#include <eldr/vulkan/common.hpp>
#include <eldr/vulkan/engine.hpp>
#include <eldr/vulkan/imgui.hpp>
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
#include <imgui.h>

#include <memory>
#include <string>

namespace eldr::vk {
struct UniformBufferObject {
  ELDR_IMPORT_CORE_TYPES();
  alignas(16) Mat4f mvp_mat;
};

VulkanEngine::VulkanEngine(const Window& window)
  : window_(window), log_(core::requestLogger("vulkan-engine")),
    in_flight_cmd_bufs_(
      std::vector<const wr::CommandBuffer*>(max_frames_in_flight, nullptr))
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
  // Create uniform buffers and descriptors
  // ---------------------------------------------------------------------------
  createUniformBuffers();
  createResourceDescriptors();
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

  core::Bitmap bitmap(filepath);
  if (bitmap.pixelFormat() != core::Bitmap::PixelFormat::RGBA)
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
    uniform_buffers_.emplace_back(
      *device_, sizeof(UniformBufferObject), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
      VMA_MEMORY_USAGE_CPU_TO_GPU, "Uniform buffer");
  }
}

void VulkanEngine::createResourceDescriptors()
{
  wr::DescriptorBuilder descriptor_builder(*device_);
  for (size_t i = 0; i < max_frames_in_flight; ++i) {
    descriptor_builder
      .addUniformBuffer<UniformBufferObject>(uniform_buffers_[i].get(), 0)
      .addCombinedImageSampler(textures_[0].sampler(), textures_[0].imageView(),
                               1);
    descriptors_.emplace_back(descriptor_builder.build(fmt::format(
      "{} (frame {})", textures_[0].name(), i))); // TODO: textures created with
                                                  // bitmaps are named
                                                  // "undefined" by default. Get
                                                  // name some other way.
  }
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
    "MSAA color buffer", TextureUsage::color_buffer, swapchain_->imageFormat(),
    sample_count);

  auto* depth_buffer = render_graph_->add<TextureResource>(
    "depth buffer", TextureUsage::depth_stencil_buffer,
    device_->findDepthFormat(), sample_count);

  // TODO: handle resolve buffers implicitly in render graph
  back_buffer_ = render_graph_->add<TextureResource>(
    "back buffer", TextureUsage::back_buffer, swapchain_->imageFormat());

  index_buffer_ = render_graph_->add<BufferResource>("index buffer",
                                                     BufferUsage::index_buffer);
  index_buffer_->uploadData<uint32_t>(indices_);

  vertex_buffer_ = render_graph_->add<BufferResource>(
    "vertex buffer", BufferUsage::vertex_buffer);
  vertex_buffer_->addVertexAttribute(VK_FORMAT_R32G32B32_SFLOAT,
                                     offsetof(GpuVertex, pos));
  vertex_buffer_->addVertexAttribute(VK_FORMAT_R32G32_SFLOAT,
                                     offsetof(GpuVertex, uv));
  vertex_buffer_->addVertexAttribute(VK_FORMAT_R32G32B32_SFLOAT,
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
      cb.bindDescriptorSets(descriptors_[current_frame_].descriptorSets(),
                            physical.pipelineLayout());
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
      cb.drawIndexed(static_cast<uint32_t>(indices_.size()));
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
  static core::StopWatch stop_watch;
  float                  time{ stop_watch.seconds(false) };

  Mat4f model{ glm::rotate(Mat4f(1.0f), time * glm::radians(20.0f),
                           Vec3f(0.0f, 0.0f, 1.0f)) };
  Mat4f view{ glm::lookAt(Vec3f(2.0f, 2.0f, 2.0f), Vec3f(0.0f, 0.0f, 0.0f),
                          Vec3f(0.0f, 0.0f, 1.0f)) };
  Mat4f proj{ glm::perspective(glm::radians(45.0f),
                               swapchain_->extent().width /
                                 (float) swapchain_->extent().height,
                               0.1f, 10.0f) };
  proj[1][1] *= -1;
  UniformBufferObject ubo{ .mvp_mat = proj * view * model };
  uniform_buffers_[current_image].uploadData(&ubo, sizeof(ubo));
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
  imgui_overlay_->update(current_frame_);
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
    GpuVertex v{ positions[i], texcoords[i], { 1.0f, 1.0f, 1.0f } };
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
  if (swapchain_invalidated_) {
    recreateSwapchain();
    swapchain_invalidated_ = false;
    return;
  }

  // Wait until the previous command buffer with the current frame index
  // has finished executing
  if (likely(in_flight_cmd_bufs_[current_frame_] != nullptr))
    in_flight_cmd_bufs_[current_frame_]->waitFence();

  const uint32_t image_index{ swapchain_->acquireNextImage(
    current_frame_, swapchain_invalidated_) };
  if (swapchain_invalidated_) {
    // Skip rendering, swapchain is recreated on next call
    return;
  }

  const auto& cb{ device_->requestCommandBuffer() };
  in_flight_cmd_bufs_[current_frame_] = &cb;
  render_graph_->render(image_index, cb);

  updateUniformBuffer(current_frame_); // move

  VkPipelineStageFlags wait_stages[]{
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

  VkSwapchainKHR         swapchains[]{ swapchain_->get() };
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

  swapchain_->present(present_info, swapchain_invalidated_);

  current_frame_ = (current_frame_ + 1) % max_frames_in_flight;
}

std::string VulkanEngine::deviceName() const { return device_->name(); }

} // namespace eldr::vk
