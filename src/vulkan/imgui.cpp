#include <eldr/vulkan/descriptorallocator.hpp>
#include <eldr/vulkan/descriptorsetlayoutbuilder.hpp>
#include <eldr/vulkan/descriptorwriter.hpp>
#include <eldr/vulkan/imgui.hpp>
#include <eldr/vulkan/pipelinebuilder.hpp>
#include <eldr/vulkan/rendergraph.hpp>
#include <eldr/vulkan/wrappers/commandbuffer.hpp>
#include <eldr/vulkan/wrappers/device.hpp>
#include <eldr/vulkan/wrappers/shader.hpp>
#include <eldr/vulkan/wrappers/texture.hpp>

namespace eldr::vk {
ImGuiOverlay::ImGuiOverlay(const wr::Device&    device,
                           const wr::Swapchain& swapchain,
                           RenderGraph*         render_graph,
                           TextureResource*     back_buffer)
  : device_(device), swapchain_(swapchain)
{
  IMGUI_CHECKVERSION();
  log_->trace("Creating ImGUI context");
  ImGui::CreateContext();

  // ImGui style
  // TODO: make it look nice!
  ImGuiStyle& style                       = ImGui::GetStyle();
  style.Colors[ImGuiCol_TitleBg]          = ImVec4(0.8f, 0.19f, 0.01f, 1.0f);
  style.Colors[ImGuiCol_TitleBgActive]    = ImVec4(0.8f, 0.19f, 0.01f, 1.0f);
  style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.8f, 0.19f, 0.01f, 0.1f);
  style.Colors[ImGuiCol_MenuBarBg]        = ImVec4(0.8f, 0.19f, 0.01f, 0.4f);
  style.Colors[ImGuiCol_Header]           = ImVec4(0.8f, 0.19f, 0.01f, 0.4f);
  style.Colors[ImGuiCol_HeaderActive]     = ImVec4(0.8f, 0.19f, 0.01f, 0.4f);
  style.Colors[ImGuiCol_HeaderHovered]    = ImVec4(0.8f, 0.19f, 0.01f, 0.4f);
  style.Colors[ImGuiCol_FrameBg]          = ImVec4(0.0f, 0.0f, 0.0f, 0.8f);
  style.Colors[ImGuiCol_CheckMark]        = ImVec4(0.8f, 0.19f, 0.01f, 0.8f);
  style.Colors[ImGuiCol_SliderGrab]       = ImVec4(0.8f, 0.19f, 0.01f, 0.4f);
  style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.8f, 0.19f, 0.01f, 0.8f);
  style.Colors[ImGuiCol_FrameBgHovered]   = ImVec4(1.0f, 1.0f, 1.0f, 0.1f);
  style.Colors[ImGuiCol_FrameBgActive]    = ImVec4(1.0f, 1.0f, 1.0f, 0.2f);
  style.Colors[ImGuiCol_Button]           = ImVec4(0.8f, 0.19f, 0.01f, 0.4f);
  style.Colors[ImGuiCol_ButtonHovered]    = ImVec4(0.8f, 0.19f, 0.01f, 0.6f);
  style.Colors[ImGuiCol_ButtonActive]     = ImVec4(0.8f, 0.19f, 0.01f, 0.8f);

  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.FontGlobalScale = scale_;
  // style.ScaleAllSizes(scale_);

  log_->trace("Loading ImGUI shaders");
  vertex_shader_ = wr::Shader{ device_, "ImGUI vertex shader", "imgui.vert.spv",
                               VK_SHADER_STAGE_VERTEX_BIT };
  fragment_shader_ =
    wr::Shader{ device_, "ImGUI fragment shader", "imgui.frag.spv",
                VK_SHADER_STAGE_FRAGMENT_BIT };

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

  // Our font textures always have 4 channels and a single mip level by
  // definition.
  constexpr uint32_t font_texture_channels{ 4 };
  constexpr uint32_t font_mip_levels{ 1 };

  if (im_font == nullptr || font_texture_data == nullptr) {
    log_->error("Unable to load font {}.  Falling back to error texture",
                font_file_path);
    imgui_texture_ =
      wr::Texture{ device_, Bitmap::createCheckerboard(), font_mip_levels };
  }
  else {
    log_->trace("Creating ImGUI font texture");

    imgui_texture_ = wr::Texture{
      device_,
      Bitmap{ "ImGUI font texture",
              Bitmap::PixelFormat::RGBA,
              Struct::Type::UInt8,
              { font_texture_width, font_texture_height },
              font_texture_channels,
              {},
              font_texture_data },
      font_mip_levels,
    };
  }
  font_sampler_ = wr::Sampler{ device_, VK_FILTER_LINEAR, VK_FILTER_LINEAR,
                               VK_SAMPLER_MIPMAP_MODE_LINEAR, font_mip_levels };

  index_buffer_  = render_graph->add<BufferResource>("imgui index buffer",
                                                     BufferUsage::IndexBuffer);
  vertex_buffer_ = render_graph->add<BufferResource>("imgui vertex buffer",
                                                     BufferUsage::VertexBuffer);
  vertex_buffer_->addVertexAttribute(VK_FORMAT_R32G32_SFLOAT,
                                     offsetof(ImDrawVert, pos));
  vertex_buffer_->addVertexAttribute(VK_FORMAT_R32G32_SFLOAT,
                                     offsetof(ImDrawVert, uv));
  vertex_buffer_->addVertexAttribute(VK_FORMAT_R8G8B8A8_UNORM,
                                     offsetof(ImDrawVert, col));

  stage_ = render_graph->add<GraphicsStage>("imgui stage");
  stage_->writesTo(back_buffer);
  stage_->readsFrom(index_buffer_);
  stage_->readsFrom(vertex_buffer_);
  // TODO: what to do with imgui pipeline?
  //  stage_->bindBuffer(vertex_buffer_, 0);
  //  stage_->usesShader(vertex_shader_);
  //  stage_->usesShader(fragment_shader_);
  //  stage_->setCullMode(VK_CULL_MODE_NONE);

  // stage_->addDescriptorLayout(set_layout_);

  buildPipeline();

  // Setup blend attachment.
  // stage_->setBlendAttachment({
  //   .blendEnable         = VK_TRUE,
  //   .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
  //   .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
  //   .colorBlendOp        = VK_BLEND_OP_ADD,
  //   .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
  //   .dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
  //   .alphaBlendOp        = VK_BLEND_OP_ADD,
  //   .colorWriteMask      = {},
  // });
}

ImGuiOverlay::~ImGuiOverlay() { ImGui::DestroyContext(); }

void ImGuiOverlay::buildPipeline()
{
  // Setup push constant range for global translation and scale.
  const VkPushConstantRange imgui_range{
    .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
    .offset     = 0,
    .size       = sizeof(PushConstantBlock),
  };

  DescriptorSetLayoutBuilder descriptor_builder;
  for (uint32_t i = 0; i < max_frames_in_flight; ++i) {
    descriptor_builder.addCombinedImageSampler(0, VK_SHADER_STAGE_FRAGMENT_BIT);
  }
  imgui_layout_ = descriptor_builder.build(device_);

  wr::Shader vert_shader{ device_, "ImGui vertex shader", "imgui.vert.spv",
                          VK_SHADER_STAGE_VERTEX_BIT };
  wr::Shader frag_shader{ device_, "ImGui fragment shader", "imgui.frag.spv",
                          VK_SHADER_STAGE_FRAGMENT_BIT };
  PipelineBuilder pipeline_builder;
  pipeline_builder.addDescriptorSetLayout(imgui_layout_)
    .addPushConstantRange(imgui_range)
    .setShaders(vert_shader, frag_shader)
    .setInputTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
    .setPolygonMode(VK_POLYGON_MODE_FILL)
    .setCullMode(VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE)
    .setMultisamplingNone()
    .disableBlending()
    .enableDepthtest(
      true, VK_COMPARE_OP_GREATER_OR_EQUAL) // TODO: not the same compare op
                                            // as i used in rendergraph

    // TODO: THE FORMATS SHOULD BE SET FROM TEXTURE RESOURCE
    // render format
    //.setColorAttachmentFormat(back_buffer_->format_)
    //.setDepthFormat(depth_buffer->format_);
    .setColorAttachmentFormat(swapchain_.imageFormat())
    .setDepthFormat(device_.findDepthFormat());

  // finally build the pipeline
  imgui_pipeline_ = pipeline_builder.build(device_, "ImGui pipeline");
}

void ImGuiOverlay::update(DescriptorAllocator& descriptors)
{
  ImDrawData* imgui_draw_data = ImGui::GetDrawData();
  if (imgui_draw_data == nullptr) {
    return;
  }

  if (imgui_draw_data->TotalIdxCount == 0 ||
      imgui_draw_data->TotalVtxCount == 0) {
    return;
  }

  std::vector<uint32_t> index_data;
  // TODO: there is room for improvement here. Data could be more efficiently
  // appended to the BufferResource without using a vector in between.
  for (int i = 0; i < imgui_draw_data->CmdListsCount; i++) {
    const ImDrawList* cmd_list = imgui_draw_data->CmdLists[i];
    for (int j = 0; j < cmd_list->IdxBuffer.Size; j++) {
      index_data.push_back(cmd_list->IdxBuffer.Data[j]);
    }
  }
  index_buffer_->bindData(index_data);

  std::vector<ImDrawVert> vertex_data;
  for (int i = 0; i < imgui_draw_data->CmdListsCount; i++) {
    const ImDrawList* cmd_list = imgui_draw_data->CmdLists[i];
    for (int j = 0; j < cmd_list->VtxBuffer.Size; j++) {
      vertex_data.push_back(cmd_list->VtxBuffer.Data[j]);
    }
  }
  vertex_buffer_->bindData(vertex_data);

  stage_->setOnRecord([&](const PhysicalStage&     physical,
                          const wr::CommandBuffer& cb) {
    ImDrawData* imgui_draw_data = ImGui::GetDrawData();
    if (imgui_draw_data == nullptr) {
      return;
    }

    const ImGuiIO& io = ImGui::GetIO();
    push_const_block_.scale =
      Vec2f(2.0f / io.DisplaySize.x, 2.0f / io.DisplaySize.y);
    push_const_block_.translate = Vec2f(-1.0f);
    VkDescriptorSet  descriptor_set{ descriptors.allocate(device_,
                                                          imgui_layout_) };
    DescriptorWriter writer;
    writer.writeCombinedImageSampler(1, imgui_texture_, font_sampler_)
      .updateSet(device_, descriptor_set);

    VkDescriptorSet im_descriptors[]{ descriptor_set };
    cb.bindPipeline(imgui_pipeline_);
    cb.bindDescriptorSets(im_descriptors, imgui_pipeline_.layout());
    cb.pushConstants(imgui_pipeline_.layout(), VK_SHADER_STAGE_VERTEX_BIT,
                     sizeof(PushConstantBlock), &push_const_block_);

    const VkViewport viewports[] = { {
      .x        = 0.0f,
      .y        = 0.0f,
      .width    = ImGui::GetIO().DisplaySize.x,
      .height   = ImGui::GetIO().DisplaySize.y,
      .minDepth = 0.0f,
      .maxDepth = 1.0f,
    } };
    cb.setViewport(viewports, 0);

    uint32_t index_offset  = 0;
    int32_t  vertex_offset = 0;
    for (int i = 0; i < imgui_draw_data->CmdListsCount; i++) {
      const ImDrawList* cmd_list = imgui_draw_data->CmdLists[i];
      for (int j = 0; j < cmd_list->CmdBuffer.Size; j++) {
        const ImDrawCmd& draw_cmd   = cmd_list->CmdBuffer[j];
        const VkRect2D   scissors[] = { {
            .offset = { std::max(static_cast<int32_t>(draw_cmd.ClipRect.x), 0),
                        std::max(static_cast<int32_t>(draw_cmd.ClipRect.y), 0) },
            .extent = { static_cast<uint32_t>(draw_cmd.ClipRect.z -
                                              draw_cmd.ClipRect.x),
                        static_cast<uint32_t>(draw_cmd.ClipRect.w -
                                              draw_cmd.ClipRect.y) },
        } };
        cb.setScissor(scissors, 0);
        cb.drawIndexed(draw_cmd.ElemCount, 1, index_offset, vertex_offset, 0);
        index_offset += draw_cmd.ElemCount;
      }
      vertex_offset += cmd_list->VtxBuffer.Size;
    }
  });
}
} // namespace eldr::vk
