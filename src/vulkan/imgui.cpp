#include <eldr/core/logger.hpp>
#include <eldr/vulkan/imgui.hpp>
#include <eldr/vulkan/rendergraph.hpp>
#include <eldr/vulkan/wrappers/commandbuffer.hpp>
#include <eldr/vulkan/wrappers/descriptor.hpp>
#include <eldr/vulkan/wrappers/descriptorbuilder.hpp>
#include <eldr/vulkan/wrappers/gputexture.hpp>
#include <eldr/vulkan/wrappers/shader.hpp>

namespace eldr::vk {
ImGuiOverlay::ImGuiOverlay(const wr::Device&    device,
                           const wr::Swapchain& swapchain,
                           RenderGraph*         render_graph,
                           TextureResource*     back_buffer)
  : device_(device), swapchain_(swapchain),
    log_(detail::requestLogger("vulkan-engine"))
{
  IMGUI_CHECKVERSION();
  log_->trace("Creating ImGUI context");
  ImGui::CreateContext();

  // ImGui style
  // TODO: make it look nice!
  ImGuiStyle& style                       = ImGui::GetStyle();
  style.Colors[ImGuiCol_TitleBg]          = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
  style.Colors[ImGuiCol_TitleBgActive]    = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
  style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.0f, 0.0f, 0.0f, 0.1f);
  style.Colors[ImGuiCol_MenuBarBg]        = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
  style.Colors[ImGuiCol_Header]           = ImVec4(0.8f, 0.0f, 0.0f, 0.4f);
  style.Colors[ImGuiCol_HeaderActive]     = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
  style.Colors[ImGuiCol_HeaderHovered]    = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
  style.Colors[ImGuiCol_FrameBg]          = ImVec4(0.0f, 0.0f, 0.0f, 0.8f);
  style.Colors[ImGuiCol_CheckMark]        = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);
  style.Colors[ImGuiCol_SliderGrab]       = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
  style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);
  style.Colors[ImGuiCol_FrameBgHovered]   = ImVec4(1.0f, 1.0f, 1.0f, 0.1f);
  style.Colors[ImGuiCol_FrameBgActive]    = ImVec4(1.0f, 1.0f, 1.0f, 0.2f);
  style.Colors[ImGuiCol_Button]           = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
  style.Colors[ImGuiCol_ButtonHovered]    = ImVec4(1.0f, 0.0f, 0.0f, 0.6f);
  style.Colors[ImGuiCol_ButtonActive]     = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);

  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.FontGlobalScale = scale_;

  log_->trace("Loading ImGUI shaders");
  vertex_shader_ =
    std::make_unique<wr::Shader>(device_, VK_SHADER_STAGE_VERTEX_BIT,
                                 "ImGUI vertex shader", "imgui.vert.spv");
  fragment_shader_ =
    std::make_unique<wr::Shader>(device_, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 "ImGUI fragment shader", "imgui.frag.spv");

  // Load font texture
  // TODO: Move this data into a container class; have container class also
  // support bold and italic.
  const char* env_p = std::getenv("ELDR_DIR");
  if (env_p == nullptr) {
    Throw("loadShader(): Environment not set up correctly");
  }
  constexpr const char* font      = "Cabin/Cabin-Bold.ttf";
  constexpr float       font_size = 18.0f;
  std::string           font_file_path =
    fmt::format("{}/assets/fonts/{}", std::string(env_p), font);

  log_->trace("Loading font {}", font_file_path);

  ImFont* im_font =
    io.Fonts->AddFontFromFileTTF(font_file_path.c_str(), font_size);

  uint8_t* font_texture_data{};
  int      font_texture_width{ 0 };
  int      font_texture_height{ 0 };
  io.Fonts->GetTexDataAsRGBA32(&font_texture_data, &font_texture_width,
                               &font_texture_height);

  if (im_font == nullptr || font_texture_data == nullptr) {
    log_->error("Unable to load font {}.  Falling back to error texture",
                font_file_path);
    imgui_texture_ = std::make_unique<wr::GpuTexture>(device_, Bitmap());
  }
  else {
    log_->trace("Creating ImGUI font texture");

    // Our font textures always have 4 channels and a single mip level by
    // definition.
    constexpr uint32_t font_texture_channels{ 4 };
    constexpr uint32_t font_mip_levels{ 1 };

    VkDeviceSize upload_size = static_cast<VkDeviceSize>(font_texture_width) *
                               static_cast<VkDeviceSize>(font_texture_height) *
                               static_cast<VkDeviceSize>(font_texture_channels);

    imgui_texture_ = std::make_unique<wr::GpuTexture>(
      device_, font_texture_data, upload_size,
      static_cast<uint32_t>(font_texture_width),
      static_cast<uint32_t>(font_texture_height), font_texture_channels,
      font_mip_levels, "ImGUI font texture");
  }

  // Create an instance of the resource descriptor builder.
  // This allows us to make resource descriptors with the help of a builder
  // pattern.
  wr::DescriptorBuilder descriptor_builder(device_);

  // Make use of the builder to create a resource descriptor for the combined
  // image sampler.
  descriptor_ = std::make_unique<wr::ResourceDescriptor>(
    descriptor_builder
      .addCombinedImageSampler(imgui_texture_->sampler(),
                               imgui_texture_->imageView(), 0)
      .build("ImGUI"));

  index_buffer_  = render_graph->add<BufferResource>("imgui index buffer",
                                                     BufferUsage::index_buffer);
  vertex_buffer_ = render_graph->add<BufferResource>(
    "imgui vertex buffer", BufferUsage::vertex_buffer);
  vertex_buffer_->addVertexAttribute(VK_FORMAT_R32G32_SFLOAT,
                                     offsetof(ImDrawVert, pos));
  vertex_buffer_->addVertexAttribute(VK_FORMAT_R8G8B8A8_UNORM,
                                     offsetof(ImDrawVert, col));
  vertex_buffer_->addVertexAttribute(VK_FORMAT_R32G32_SFLOAT,
                                     offsetof(ImDrawVert, uv));
  vertex_buffer_->setElementSize(sizeof(ImDrawVert));

  stage_ = render_graph->add<GraphicsStage>("imgui stage");
  stage_->writesTo(back_buffer);
  stage_->readsFrom(index_buffer_);
  stage_->readsFrom(vertex_buffer_);
  stage_->bindBuffer(vertex_buffer_, 0);
  stage_->usesShader(*vertex_shader_);
  stage_->usesShader(*fragment_shader_);

  stage_->addDescriptorLayout(descriptor_->descriptorSetLayout());

  // Setup push constant range for global translation and scale.
  stage_->addPushConstantRange({
    .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
    .offset     = 0,
    .size       = sizeof(PushConstantBlock),
  });

  // Setup blend attachment.
  stage_->setBlendAttachment({
    .blendEnable         = VK_TRUE,
    .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
    .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
    .colorBlendOp        = VK_BLEND_OP_ADD,
    .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
    .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
    .alphaBlendOp        = VK_BLEND_OP_ADD,
    .colorWriteMask      = {},
  });
}

ImGuiOverlay::~ImGuiOverlay() { ImGui::DestroyContext(); }

void ImGuiOverlay::update()
{
  ImDrawData* imgui_draw_data = ImGui::GetDrawData();
  if (imgui_draw_data == nullptr) {
    return;
  }

  if (imgui_draw_data->TotalIdxCount == 0 ||
      imgui_draw_data->TotalVtxCount == 0) {
    return;
  }

  bool should_update = false;
  if (static_cast<int>(index_data_.size()) != imgui_draw_data->TotalIdxCount) {
    index_data_.clear();
    for (size_t i = 0; static_cast<int>(i) < imgui_draw_data->CmdListsCount;
         i++) {
      const ImDrawList* cmd_list = imgui_draw_data->CmdLists[i];
      for (size_t j = 0; static_cast<int>(j) < cmd_list->IdxBuffer.Size; j++) {
        index_data_.push_back(cmd_list->IdxBuffer.Data[j]);
      }
    }
    index_buffer_->uploadData<uint32_t>(index_data_);
    should_update = true;
  }

  if (static_cast<int>(vertex_data_.size()) != imgui_draw_data->TotalVtxCount) {
    vertex_data_.clear();
    for (size_t i = 0; static_cast<int>(i) < imgui_draw_data->CmdListsCount;
         i++) {
      const ImDrawList* cmd_list = imgui_draw_data->CmdLists[i];
      for (std::size_t j = 0; static_cast<int>(j) < cmd_list->VtxBuffer.Size;
           j++) {
        vertex_data_.push_back(cmd_list->VtxBuffer.Data[j]);
      }
    }
    vertex_buffer_->uploadData<ImDrawVert>(vertex_data_);
    should_update = true;
  }

  if (!should_update) {
    return;
  }

  stage_->setOnRecord([this](const PhysicalStage&     physical,
                             const wr::CommandBuffer& cmd_buf) {
    ImDrawData* imgui_draw_data = ImGui::GetDrawData();
    if (imgui_draw_data == nullptr) {
      return;
    }

    const ImGuiIO& io = ImGui::GetIO();
    push_const_block_.scale =
      Vec2f(2.0f / io.DisplaySize.x, 2.0f / io.DisplaySize.y);
    push_const_block_.translate = Vec2f(-1.0f);
    cmd_buf.bindDescriptorSets(descriptor_->descriptorSets(),
                               physical.pipelineLayout());
    cmd_buf.pushConstants(physical.pipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT,
                          sizeof(PushConstantBlock), &push_const_block_);

    uint32_t index_offset  = 0;
    int32_t  vertex_offset = 0;
    for (size_t i = 0; static_cast<int>(i) < imgui_draw_data->CmdListsCount;
         i++) {
      const ImDrawList* cmd_list = imgui_draw_data->CmdLists[i];
      for (std::int32_t j = 0; j < cmd_list->CmdBuffer.Size; j++) {
        const ImDrawCmd& draw_cmd = cmd_list->CmdBuffer[j];
        vkCmdDrawIndexed(cmd_buf.get(), draw_cmd.ElemCount, 1, index_offset,
                         vertex_offset, 0);
        index_offset += draw_cmd.ElemCount;
      }
      vertex_offset += cmd_list->VtxBuffer.Size;
    }
  });
}
} // namespace eldr::vk
