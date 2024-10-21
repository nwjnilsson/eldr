// Ensure that vma implementation is included
#include "eldr/vulkan/wrappers/commandpool.hpp"
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#include <eldr/core/fwd.hpp>
#include <eldr/core/logger.hpp>
#include <eldr/core/math.hpp>
#include <eldr/vulkan/common.hpp>
#include <eldr/vulkan/engine.hpp>
#include <eldr/vulkan/helpers.hpp>
#include <eldr/vulkan/vertex.hpp>
#include <eldr/vulkan/wrappers/device.hpp>
#include <eldr/vulkan/wrappers/fence.hpp>

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>
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

// -----------------------------------------------------------------------------
// fwd
// -----------------------------------------------------------------------------
#ifdef ELDR_VULKAN_DEBUG_REPORT
static VkDebugUtilsMessengerEXT
setupDebugMessenger(VkInstance instance, VkAllocationCallbacks* allocator);
#endif
// -----------------------------------------------------------------------------
VulkanEngine::VulkanEngine(uint32_t width, uint32_t height)
  : window_(std::make_unique<wr::Window>(width, height)),
#ifdef ELDR_VULKAN_DEBUG_REPORT
    debug_messenger_(setupDebugMessenger(instance_.get(), nullptr)),
#endif
    texture_sampler_(&device_, command_pool_), vertices_(), indices_(),
    vertex_buffer_(&device_, vertices_, command_pool_),
    index_buffer_(&device_, indices_, command_pool_),
    uniform_buffers_(max_frames_in_flight),
    uniform_buffers_mapped_(max_frames_in_flight),
    command_buffers_(max_frames_in_flight),
    // command_buffers_(max_frames_in_flight),
    descriptor_sets_(max_frames_in_flight)
{
  // ---------------------------------------------------------------------------
  // Create instance
  // ---------------------------------------------------------------------------
  auto app_info             = wr::makeInfo<VkApplicationInfo>();
  app_info.pApplicationName = app_name;
  app_info.applicationVersion =
    VK_MAKE_API_VERSION(0, app_version[0], app_version[1], app_version[2]);
  app_info.pEngineName   = engine_name;
  app_info.engineVersion = VK_MAKE_API_VERSION(
    engine_version[0], engine_version[1], engine_version[2], engine_version[3]);
  app_info.apiVersion = VK_API_VERSION_1_3;

  instance_ = std::make_unique<wr::Instance>(app_info, instance_extensions);

  // ---------------------------------------------------------------------------
  // Create surface
  // ---------------------------------------------------------------------------
  surface_ = std::make_unique<wr::Surface>(instance_, glfw_window_);

  // ---------------------------------------------------------------------------
  // Create device
  // ---------------------------------------------------------------------------
  std::vector<const std::string> device_extensions;
  device_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
#ifdef VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME
  device_extensions.push_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
#endif
  device_ =
    std::make_unique<wr::Device>(instance_, surface_, device_extensions);

  // ---------------------------------------------------------------------------
  // Create swapchain
  // ---------------------------------------------------------------------------
  auto swapchain_support = device_->getSwapchainSupportDetails(surface);
  auto extent      = window.selectSwapExtent(support_details.capabilities);
  const swapchain_ = std::make_unique<wr::Swapchain>(
    device_->logical(), surface_->get(), extent, swapchain_support);

  createUniformBuffers();

  createCommandBuffers();

  // ---------------------------------------------------------------------------
  // Create resource descriptors
  // ---------------------------------------------------------------------------
  createResourceDescriptors();

  queue_family_indices = wr::findQueueFamilies(device_, surface_);
  // ---------------------------------------------------------------------------
  // Create render graph (?)
  // ---------------------------------------------------------------------------

  // ---------------------------------------------------------------------------
  // Create command pool
  // ---------------------------------------------------------------------------
  command_pool_(std::make_unique<wr::CommandPool>(*device_)),

  // Sync objects
  for (uint8_t i = 0; i < max_frames_in_flight; ++i) {
    image_available_sem_.emplace_back(Semaphore(&device_));
    render_finished_sem_.emplace_back(Semaphore(&device_));
    in_flight_fences_.emplace_back(Fence(&device_));
  }
}

VulkanEngine::~VulkanEngine()
{
  vkDeviceWaitIdle(device_.logical());
#ifdef ELDR_VULKAN_DEBUG_REPORT
  auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(
    instance_.get(), "vkDestroyDebugUtilsMessengerEXT");
  if (func != nullptr)
    func(instance_.get(), debug_messenger_, nullptr);
#endif
}

#ifdef ELDR_VULKAN_DEBUG_REPORT
static VKAPI_ATTR VkBool32 VKAPI_CALL vkDebugReportCallback(
  VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
  VkDebugUtilsMessageTypeFlagsEXT             messageType,
  const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
  (void) pUserData; // TODO: figure out how to use
  std::string type{};
  switch (messageType) {
    case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
      type = "general";
      break;
    case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
      type = "validation";
      break;
    case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
      type = "performance";
      break;
    case VK_DEBUG_UTILS_MESSAGE_TYPE_FLAG_BITS_MAX_ENUM_EXT:
      type = "flag bits max"; // what even
      break;
    default:
      type = "invalid";
      break;
  }

  switch (messageSeverity) {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
      spdlog::debug("[VULKAN DBG ({})]: {}", pCallbackData->pMessage, type);
      break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
      spdlog::info("[VULKAN DBG ({})]: {}", pCallbackData->pMessage, type);
      break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
      spdlog::warn("[VULKAN DBG ({})]: {}", pCallbackData->pMessage, type);
      break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
      spdlog::error("[VULKAN DBG ({})]: {}", pCallbackData->pMessage, type);
      break;
    default:
      break;
  }
  return VK_FALSE;
}

VkDebugUtilsMessengerEXT setupDebugMessenger(VkInstance             instance,
                                             VkAllocationCallbacks* allocator)
{
  VkDebugUtilsMessengerEXT debug_messenger;
  // Debug report callback
  VkDebugUtilsMessengerCreateInfoEXT debug_report_ci{};

  debug_report_ci.sType =
    VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  debug_report_ci.messageSeverity =
    VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
    VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
    VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  debug_report_ci.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  debug_report_ci.pfnUserCallback = vkDebugReportCallback;
  debug_report_ci.pUserData       = nullptr; // Optional

  auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(
    instance, "vkCreateDebugUtilsMessengerEXT");
  assert(func != nullptr);
  VkResult err = func(instance, &debug_report_ci, allocator, &debug_messenger);
  if (err != VK_SUCCESS)
    ThrowVk("Failed to create debug utils messenger!");

  return debug_messenger;
}
#endif

bool VulkanEngine::isFrameBufferResized()
{
  return window_->extent() != swapchain_->extent();
}

void VulkanEngine::createUniformBuffers()
{
  BufferInfo   buffer_info{ .size       = sizeof(UniformBufferObject),
                            .usage      = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                            .properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT };
  VkDeviceSize buffer_size = sizeof(UniformBufferObject);
  for (uint8_t i = 0; i < max_frames_in_flight; ++i) {
    uniform_buffers_[i] = Buffer(&device_, buffer_info);
    vkMapMemory(device_.logical(), uniform_buffers_[i].memory(), 0, buffer_size,
                0, &uniform_buffers_mapped_[i]);
  }
}

void VulkanEngine::createCommandBuffers()
{
  for (uint32_t i = 0; i < max_frames_in_flight; ++i) {
    command_buffers_[i] = CommandBuffer(&device_, &command_pool_);
  }
}

void VulkanEngine::createResourceDescriptors()
{
  VkDescriptorSetLayoutBinding ubo_layout_binding{};
  ubo_layout_binding.binding            = 0;
  ubo_layout_binding.descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  ubo_layout_binding.descriptorCount    = 1;
  ubo_layout_binding.stageFlags         = VK_SHADER_STAGE_VERTEX_BIT;
  ubo_layout_binding.pImmutableSamplers = nullptr; // optional

  VkDescriptorSetLayoutBinding sampler_layout_binding{};
  sampler_layout_binding.binding         = 1;
  sampler_layout_binding.descriptorCount = 1;
  sampler_layout_binding.descriptorType =
    VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  sampler_layout_binding.pImmutableSamplers = nullptr;
  sampler_layout_binding.stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT;

  const std::vector<VkDescriptorSetLayoutBinding> bindings{
    ubo_layout_binding, sampler_layout_binding
  };

  for (size_t i = 0; i < max_frames_in_flight; ++i) {
    VkDescriptorBufferInfo buffer_info{};
    buffer_info.buffer = uniform_buffers_[i].get();
    buffer_info.offset = 0;
    buffer_info.range  = sizeof(UniformBufferObject);

    VkDescriptorImageInfo image_info{};
    image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    image_info.imageView   = texture_sampler_.texture.view().get();
    image_info.sampler     = texture_sampler_.sampler.get();

    std::array<VkWriteDescriptorSet, 2> descriptor_writes{};
    descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_writes[0].dstArrayElement = 0;
    descriptor_writes[0].descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_writes[0].descriptorCount = 1;
    // Either pBufferInfo or pImageInfo or pTexelBufferView is set
    descriptor_writes[0].pBufferInfo      = &buffer_info;
    descriptor_writes[0].pImageInfo       = nullptr;
    descriptor_writes[0].pTexelBufferView = nullptr;

    descriptor_writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_writes[1].dstArrayElement = 0;
    descriptor_writes[1].descriptorType =
      VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_writes[1].descriptorCount = 1;
    // Either pBufferInfo or pImageInfo or pTexelBufferView is set
    descriptor_writes[1].pBufferInfo      = nullptr;
    descriptor_writes[1].pImageInfo       = &image_info;
    descriptor_writes[1].pTexelBufferView = nullptr;

    descriptors_.emplace_back(device_, bindings, descriptor_writes);
  }
}

// Needed only for imgui thing
// TODO: this can be improved...
static void checkVkResult(VkResult result) { CheckVkResult(result); }

void VulkanEngine::initImGui()
{
  // setup
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  (void) io;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.Fonts->AddFontDefault();
  ImGui::StyleColorsDark();
  // TODO: set up fonts and more style stuff
  ImGui_ImplGlfw_InitForVulkan(window_, true);

  // init
  QueueFamilyIndices queue_family_indices =
    findQueueFamilies(device_.physical(), surface_.get());
  ImGui_ImplVulkan_InitInfo init_info = {};
  init_info.Instance                  = instance_.get();
  init_info.PhysicalDevice            = device_.physical();
  init_info.Device                    = device_.logical();
  init_info.QueueFamily     = queue_family_indices.graphics_family.value();
  init_info.Queue           = device_.graphicsQueue();
  init_info.PipelineCache   = VK_NULL_HANDLE;
  init_info.DescriptorPool  = descriptor_pool_.get();
  init_info.Subpass         = 0;
  init_info.MinImageCount   = swapchain_.minImageCount();
  init_info.ImageCount      = swapchain_.minImageCount();
  init_info.MSAASamples     = swapchain_.msaaSamples();
  init_info.Allocator       = nullptr;
  init_info.CheckVkResultFn = vk::checkVkResult;
  ImGui_ImplVulkan_Init(&init_info, swapchain_.render_pass_.get());

  // Upload fonts
  {
    CommandBuffer cb(&device_, &command_pool_);
    cb.beginSingleCommand();
    ImGui_ImplVulkan_CreateFontsTexture(cb.get());
    cb.submit();
    ImGui_ImplVulkan_DestroyFontUploadObjects();
  }
}

void VulkanEngine::shutdownImGui()
{
  vkDeviceWaitIdle(device_.logical());
  ImGui_ImplVulkan_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}

void VulkanEngine::record(uint32_t image_index)
{
  CommandBuffer& cb = command_buffers_[current_frame_];

  cb.begin();

  // Note that the order matters, depth is on index 1
  std::array<VkClearValue, 2> clear_values{};
  clear_values[0].color        = { { 0.0f, 0.0f, 0.0f, 1.0f } };
  clear_values[1].depthStencil = { 1.0f, 0 };

  command_buffers_[current_frame_].beginRenderPass(
    swapchain_.render_pass_.get(), swapchain_.framebuffers_[image_index],
    swapchain_.extent(), clear_values.size(), clear_values.data());

  vkCmdBindPipeline(cb.get(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_.get());

  VkViewport viewport{};
  viewport.x        = 0.0f;
  viewport.y        = 0.0f;
  viewport.width    = static_cast<float>(swapchain_.extent().width);
  viewport.height   = static_cast<float>(swapchain_.extent().height);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  vkCmdSetViewport(cb.get(), 0, 1, &viewport);

  VkRect2D scissor{};
  scissor.offset = { 0, 0 };
  scissor.extent = swapchain_.extent();
  vkCmdSetScissor(cb.get(), 0, 1, &scissor);

  vkCmdBindPipeline(cb.get(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_.get());

  VkBuffer     vertex_buffers[] = { vertex_buffer_.get() };
  VkDeviceSize offsets[]        = { 0 };

  vkCmdBindVertexBuffers(cb.get(), 0, 1, vertex_buffers, offsets);
  vkCmdBindIndexBuffer(cb.get(), index_buffer_.get(), 0, VK_INDEX_TYPE_UINT32);

  vkCmdBindDescriptorSets(cb.get(), VK_PIPELINE_BIND_POINT_GRAPHICS,
                          pipeline_.layout(), 0, 1,
                          &descriptor_sets_[current_frame_], 0, nullptr);

  vkCmdDrawIndexed(cb.get(), static_cast<uint32_t>(indices_.size()), 1, 0, 0,
                   0);

  ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cb.get());

  cb.endRenderPass();
  cb.end();
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
                               swapchain_.extent().width /
                                 (float) swapchain_.extent().height,
                               0.1f, 10.0f);
  ubo.proj[1][1] *= -1;
  memcpy(uniform_buffers_mapped_[current_image], &ubo, sizeof(ubo));
}

void VulkanEngine::submitGeometry(const std::vector<Vec3f>& positions,
                                  const std::vector<Vec2f>& texcoords)
{
  vertices_.clear();
  indices_.clear();
  std::unordered_map<Vertex, uint32_t> unique_vertices{};

  for (uint32_t i = 0; i < positions.size(); ++i) {
    Vertex v{ positions[i], { 1.0f, 1.0f, 1.0f }, texcoords[i] };
    if (unique_vertices.count(v) == 0) {
      unique_vertices[v] = static_cast<uint32_t>(vertices_.size());
      vertices_.push_back(v);
    }
    indices_.push_back(unique_vertices[v]);
  }
  vertex_buffer_ = Buffer(&device_, vertices_, command_pool_);
  index_buffer_  = Buffer(&device_, indices_, command_pool_);
}

void VulkanEngine::newFrame()
{
  ImGui_ImplVulkan_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
}
void VulkanEngine::drawFrame()
{
  in_flight_fences_[current_frame_].wait();

  uint32_t image_index;
  VkResult result = swapchain_.acquireNextImage(
    image_index, image_available_sem_[current_frame_]);

  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    swapchain_.recreate(window_);
    // VK_SUBOPTIMAL_KHR is considered a "success" so the return here should
    // be removed if we also recreate the swapchain if it were suboptimal
    return;
  }
  // but instead we present anyway if the swapchain is suboptimal since the
  // image is already acquired
  else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    ThrowVk("Failed to acquire swapchain image!");
  }

  in_flight_fences_[current_frame_].reset();
  command_buffers_[current_frame_].reset();

  record(image_index);

  updateUniformBuffer(current_frame_);

  VkSubmitInfo submit_info{};
  submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

  VkSemaphore wait_semaphores[] = {
    image_available_sem_[current_frame_].get()
  };
  VkPipelineStageFlags wait_stages[] = {
    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
  };
  submit_info.waitSemaphoreCount  = 1;
  submit_info.pWaitSemaphores     = wait_semaphores;
  submit_info.pWaitDstStageMask   = wait_stages;
  submit_info.commandBufferCount  = 1;
  submit_info.pCommandBuffers     = &command_buffers_[current_frame_].get();
  VkSemaphore signal_semaphores[] = {
    render_finished_sem_[current_frame_].get()
  };
  submit_info.signalSemaphoreCount = 1;
  submit_info.pSignalSemaphores    = signal_semaphores;

  if (vkQueueSubmit(device_.graphicsQueue(), 1, &submit_info,
                    in_flight_fences_[current_frame_].get()) != VK_SUCCESS)
    ThrowVk("Failed to submit draw command buffer!");

  VkPresentInfoKHR present_info{};
  present_info.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  present_info.waitSemaphoreCount = 1;
  present_info.pWaitSemaphores    = signal_semaphores;

  VkSwapchainKHR swapchains[] = { swapchain_.get() };
  present_info.swapchainCount = 1;
  present_info.pSwapchains    = swapchains;
  present_info.pImageIndices  = &image_index;

  result = vkQueuePresentKHR(device_.presentQueue(), &present_info);
  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
      isFrameBufferResized()) {
    swapchain_.recreate(window_);
    return;
  }
  else if (result != VK_SUCCESS) {
    ThrowVk("Failed to present swapchain image!");
  }

  current_frame_ = (current_frame_ + 1) % max_frames_in_flight;
}

} // namespace eldr::vk
