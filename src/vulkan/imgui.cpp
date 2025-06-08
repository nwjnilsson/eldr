#include <eldr/core/bitmap.hpp>
#include <eldr/math/vector.hpp>
#include <eldr/vulkan/descriptorallocator.hpp>
#include <eldr/vulkan/descriptorsetlayoutbuilder.hpp>
#include <eldr/vulkan/descriptorwriter.hpp>
#include <eldr/vulkan/imgui.hpp>
#include <eldr/vulkan/pipelinebuilder.hpp>
#include <eldr/vulkan/rendergraph.hpp>
#include <eldr/vulkan/wrappers/commandbuffer.hpp>
#include <eldr/vulkan/wrappers/device.hpp>
#include <eldr/vulkan/wrappers/shader.hpp>

using namespace eldr::core;

namespace eldr::vk {
struct ImGuiOverlay::FrameData {
  wr::Buffer<uint32_t>   index_buffer;
  wr::Buffer<ImDrawVert> vertex_buffer;
};

struct PushConstantBlock {
  using Vector = CoreAliases<float>::Vector2f;
  Vector scale;
  Vector translate;
} push_const_block_;

ImGuiOverlay::ImGuiOverlay(const wr::Device&    device,
                           const wr::Swapchain& swapchain,
                           RenderGraph* const   render_graph)
  : device_(device), swapchain_(swapchain)
{
  IMGUI_CHECKVERSION();
  Log(Trace, "Creating ImGui context");
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

  Log(Trace, "Loading ImGui shaders");
  vertex_shader_ = wr::Shader{
    device_, "ImGui vertex shader", "imgui.vert.spv", VK_SHADER_STAGE_VERTEX_BIT
  };
  fragment_shader_ = wr::Shader{ device_,
                                 "ImGui fragment shader",
                                 "imgui.frag.spv",
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

  Log(Trace, "Loading font {}", font_file_path);

  ImFont* im_font =
    io.Fonts->AddFontFromFileTTF(font_file_path.c_str(), font_size);

  uint8_t* font_texture_data;
  int      font_texture_width{ 0 };
  int      font_texture_height{ 0 };
  io.Fonts->GetTexDataAsRGBA32(
    &font_texture_data, &font_texture_width, &font_texture_height);

  // Our font textures always have 4 channels and a single mip level by
  // definition.
  constexpr uint32_t font_texture_channels{ 4 };
  constexpr uint32_t font_mip_levels{ 1 };

  if (im_font == nullptr || font_texture_data == nullptr) {
    Log(Error,
        "Unable to load font {}.  Falling back to error texture",
        font_file_path);
    imgui_texture_ =
      wr::Image{ device_, Bitmap::createCheckerboard(), font_mip_levels };
  }
  else {
    Log(Trace, "Creating ImGui font texture");

    imgui_texture_ = wr::Image{
      device_,
      Bitmap{ "ImGui font texture",
              Bitmap::PixelFormat::RGBA,
              StructType::UInt8,
              static_cast<uint32_t>(font_texture_width),
              static_cast<uint32_t>(font_texture_height),
              font_texture_channels,
              {},
              reinterpret_cast<byte_t*>(font_texture_data) },
      font_mip_levels,
    };
  }
  font_sampler_ = wr::Sampler{ device_,
                               VK_FILTER_LINEAR,
                               VK_FILTER_LINEAR,
                               VK_SAMPLER_MIPMAP_MODE_LINEAR,
                               font_mip_levels };

  stage_ = render_graph->add<GraphicsStage>("ImGui stage");
  stage_->writesTo(render_graph->backBuffer(), VK_ATTACHMENT_LOAD_OP_LOAD);

  buildPipeline();

  frames_in_flight.resize(max_frames_in_flight);

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
  descriptor_builder.addCombinedImageSampler(0, VK_SHADER_STAGE_FRAGMENT_BIT);
  imgui_layout_ = descriptor_builder.build(device_);

  wr::Shader vert_shader{
    device_, "ImGui vertex shader", "imgui.vert.spv", VK_SHADER_STAGE_VERTEX_BIT
  };
  wr::Shader      frag_shader{ device_,
                          "ImGui fragment shader",
                          "imgui.frag.spv",
                          VK_SHADER_STAGE_FRAGMENT_BIT };
  PipelineBuilder pipeline_builder;
  pipeline_builder.addDescriptorSetLayout(imgui_layout_)
    .addVertexBinding(0, sizeof(ImDrawVert))
    .addVertexAttribute(VK_FORMAT_R32G32_SFLOAT, offsetof(ImDrawVert, pos))
    .addVertexAttribute(VK_FORMAT_R32G32_SFLOAT, offsetof(ImDrawVert, uv))
    .addVertexAttribute(VK_FORMAT_R8G8B8A8_UNORM, offsetof(ImDrawVert, col))
    .addPushConstantRange(imgui_range)
    .setShaders(vert_shader, frag_shader)
    .setInputTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
    .setPolygonMode(VK_POLYGON_MODE_FILL)
    .setCullMode(VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE)
    .setMultisamplingNone()
    .enableBlendingAlphaBlend()
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
  // TODO: use gpu buffers, and also implement resizing index/vertex data is far
  // below buffer size
  auto& ibuffer = frames_in_flight[frame_index_].index_buffer;
  if (ibuffer.size() >= index_data.size())
    ibuffer.uploadData(index_data);
  else
    ibuffer = { device_,
                "imgui index buffer",
                index_data,
                VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT };

  std::vector<ImDrawVert> vertex_data;
  for (int i = 0; i < imgui_draw_data->CmdListsCount; i++) {
    const ImDrawList* cmd_list = imgui_draw_data->CmdLists[i];
    for (int j = 0; j < cmd_list->VtxBuffer.Size; j++) {
      vertex_data.push_back(cmd_list->VtxBuffer.Data[j]);
    }
  }

  auto& vbuffer = frames_in_flight[frame_index_].vertex_buffer;
  if (vbuffer.size() >= vertex_data.size())
    vbuffer.uploadData(vertex_data); // assuming cpu to gpu buffer.
  else {
    vbuffer = { device_,
                "ImGui vertex buffer",
                vertex_data,
                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT };
  }

  stage_->setOnRecord([&](const wr::CommandBuffer& cb) {
    ImDrawData* imgui_draw_data = ImGui::GetDrawData();
    if (imgui_draw_data == nullptr) {
      return;
    }
    cb.bindIndexBuffer(ibuffer);
    VkBuffer vbuffers[]{ vbuffer.vk() };
    cb.bindVertexBuffers(vbuffers);

    const ImGuiIO& io = ImGui::GetIO();
    push_const_block_.scale =
      Vector2f{ 2.0f / io.DisplaySize.x, 2.0f / io.DisplaySize.y };
    push_const_block_.translate = Vector2f{ -1.0f };
    VkDescriptorSet descriptor_set{ descriptors.allocate(device_,
                                                         imgui_layout_) };

    DescriptorWriter writer;
    writer.writeCombinedImageSampler(0, imgui_texture_, font_sampler_)
      .updateSet(device_, descriptor_set);

    VkDescriptorSet im_descriptors[]{ descriptor_set };
    cb.bindPipeline(imgui_pipeline_);
    cb.bindDescriptorSets(im_descriptors, imgui_pipeline_.layout());
    cb.pushConstants(imgui_pipeline_.layout(),
                     VK_SHADER_STAGE_VERTEX_BIT,
                     sizeof(PushConstantBlock),
                     &push_const_block_);

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
  frame_index_ = (frame_index_ + 1) % max_frames_in_flight;
}
} // namespace eldr::vk
