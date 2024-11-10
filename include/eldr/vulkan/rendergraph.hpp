#pragma once
#include <eldr/core/fwd.hpp>
#include <eldr/vulkan/common.hpp>
#include <eldr/vulkan/wrappers/pipeline.hpp>

#include <functional>
#include <memory>
#include <span>
#include <vector>

namespace eldr::vk {

class RenderGraph;
class PhysicalStage;
class PhysicalResource;

// Base class object
class RenderGraphObject {
public:
  RenderGraphObject() = default;
  RenderGraphObject(const wr::Device&);
  RenderGraphObject(const RenderGraphObject&) = delete;
  RenderGraphObject(RenderGraphObject&&)      = delete;
  virtual ~RenderGraphObject()                = default;

  RenderGraphObject& operator=(const RenderGraphObject&) = delete;
  RenderGraphObject& operator=(RenderGraphObject&&)      = delete;

  template <typename T> [[nodiscard]] T*       as();
  template <typename T> [[nodiscard]] const T* as() const;
};

class RenderResource : public RenderGraphObject {
  friend RenderGraph;

protected:
  explicit RenderResource(std::string name) : name_(std::move(name)) {}

public:
  RenderResource(const RenderResource&) = delete;
  RenderResource(RenderResource&&)      = delete;
  ~RenderResource() override            = default;

  RenderResource& operator=(const RenderResource&) = delete;
  RenderResource& operator=(RenderResource&&)      = delete;

  [[nodiscard]] const std::string& name() const { return name_; }

private:
  const std::string                 name_;
  std::shared_ptr<PhysicalResource> physical_;
};

enum class BufferUsage {
  /// @brief Specifies that the buffer will be used to input index data.
  index_buffer,

  /// @brief Specifies that the buffer will be used to input per vertex data to
  /// a vertex shader.
  vertex_buffer,
};

class BufferResource : public RenderResource {
  friend RenderGraph;

public:
  BufferResource(std::string&& name, BufferUsage usage)
    : RenderResource(name), usage_(usage)
  {
  }
  /// @brief Returns the BufferUsage of this buffer
  BufferUsage usage() const { return usage_; }

  /// @brief Specifies that element `offset` of this vertex buffer is of format
  /// `format`.
  /// @note Calling this function is only valid on buffers of type
  /// BufferUsage::VERTEX_BUFFER.
  void addVertexAttribute(VkFormat format, std::uint32_t offset);

  /// @brief Specifies the element size of the buffer upfront if data is not to
  /// be uploaded immediately.
  /// @param element_size The element size in bytes
  void setElementSize(std::size_t element_size)
  {
    element_size_ = element_size;
  }

  /// @brief Specifies the data that should be uploaded to this buffer at the
  /// start of the next frame.
  /// @param data A span of elements to upload to the buffer
  template <typename T> void uploadData(const std::span<T>& data);

  /// @brief @copybrief upload_data(const T *, std::size_t)
  /// @note This is equivalent to doing `upload_data(data.data(), data.size() *
  /// sizeof(T))`
  /// @see upload_data(const T *data, std::size_t count)
  // template <typename T> void uploadData(const std::vector<T>& data);

private:
  const BufferUsage                              usage_;
  std::vector<VkVertexInputAttributeDescription> vertex_attributes_;

  // Data to upload during render graph compilation.
  const void* data_{ nullptr };
  std::size_t data_size_{ 0 };
  bool        data_upload_needed_{ false };
  std::size_t element_size_{ 0 };
};

enum class TextureUsage {
  /// @brief Specifies that this texture is the output of the render graph.
  // TODO: Refactor back buffer system more (remove need for back_buffer texture
  // usage)
  back_buffer,

  /// @brief Specifies that this texture is an offscreen buffer. Can be used as
  /// MSAA render target.
  offscreen_buffer,

  /// @brief Specifies that this texture is a combined depth/stencil buffer.
  /// @note This may mean that this texture is completely GPU-sided and cannot
  /// be accessed by the CPU in any way.
  depth_stencil_buffer,
};

class TextureResource : public RenderResource {
  friend RenderGraph;

public:
  TextureResource(
    std::string&& name, TextureUsage usage, VkFormat format,
    VkSampleCountFlagBits msaa_sample_count = VK_SAMPLE_COUNT_1_BIT)
    : RenderResource(name), usage_(usage), format_(format),
      msaa_sample_count_(msaa_sample_count)
  {
  }

private:
  const TextureUsage          usage_;
  const VkFormat              format_{ VK_FORMAT_UNDEFINED };
  const VkSampleCountFlagBits msaa_sample_count_{ VK_SAMPLE_COUNT_1_BIT };
};

/// @brief A single render stage in the render graph.
/// @note Not to be confused with a vulkan render pass.
class RenderStage : public RenderGraphObject {
  friend RenderGraph;
  friend wr::Pipeline;

public:
  RenderStage(const RenderStage&) = delete;
  RenderStage(RenderStage&&)      = delete;
  ~RenderStage() override         = default;

  RenderStage& operator=(const RenderStage&) = delete;
  RenderStage& operator=(RenderStage&&)      = delete;

  /// @brief Specifies that this stage writes to `resource`.
  void writesTo(const RenderResource* resource);

  /// @brief Specifies that this stage reads from `resource`.
  void readsFrom(const RenderResource* resource);

  /// @brief Binds a descriptor set layout to this render stage.
  /// @note This function will be removed in the near future, as we are aiming
  /// for users of the API to not have to deal with descriptors at all.
  // TODO: Refactor descriptor management in the render graph
  void addDescriptorLayout(VkDescriptorSetLayout layout)
  {
    descriptor_layouts_.push_back(layout);
  }

  /// @brief Add a push constant range to this render stage.
  /// @param range The push constant range
  void addPushConstantRange(VkPushConstantRange range)
  {
    push_constant_ranges_.push_back(range);
  }

  [[nodiscard]] const std::string& name() const { return name_; }

  /// @brief Specifies a function that will be called during command buffer
  /// recording for this stage
  /// @details This function can be used to specify other vulkan commands during
  /// command buffer recording. The most common use for this is for draw
  /// commands.
  void setOnRecord(
    std::function<void(const PhysicalStage&, const wr::CommandBuffer&)>
      on_record)
  {
    on_record_ = std::move(on_record);
  }

  /// @brief Returns true if this stage has a read dependency that is **not** of
  /// index/vertex buffer type.
  /// @details Vertex and index buffers are considered inputs only, i.e no stage
  /// will write to such buffers. A read dependency on an index/vertex buffer is
  /// therefore considered non-blocking in the render graph. This comes into
  /// play when topology sorting the render graph, as there must be stages
  /// without blocking read dependencies to perform the sorting.
  bool hasBlockingRead() const;

protected:
  explicit RenderStage(std::string name) : name_(std::move(name)) {}

private:
  const std::string                  name_;
  std::unique_ptr<PhysicalStage>     physical_;
  std::vector<const RenderResource*> writes_;
  std::vector<const RenderResource*> reads_;

  std::vector<VkDescriptorSetLayout> descriptor_layouts_;
  std::vector<VkPushConstantRange>   push_constant_ranges_;
  std::function<void(const PhysicalStage&, const wr::CommandBuffer&)>
    on_record_{ [](auto&, auto&) {} };
};

class GraphicsStage : public RenderStage {
  friend RenderGraph;
  friend wr::Pipeline;

public:
  explicit GraphicsStage(std::string&& name) : RenderStage(name) {}
  GraphicsStage(const GraphicsStage&) = delete;
  GraphicsStage(GraphicsStage&&)      = delete;
  ~GraphicsStage() override           = default;

  GraphicsStage& operator=(const GraphicsStage&) = delete;
  GraphicsStage& operator=(GraphicsStage&&)      = delete;

  /// @brief Specifies that this stage should clear the screen before rendering.
  void setClearsScreen(bool clears_screen) { clears_screen_ = clears_screen; }

  /// @brief Specifies the depth options for this stage.
  /// @param depth_test Whether depth testing should be performed
  /// @param depth_write Whether depth writing should be performed
  void setDepthOptions(bool depth_test, bool depth_write)
  {
    depth_test_  = depth_test;
    depth_write_ = depth_write;
  }

  /// @brief Set the blend attachment for this stage.
  /// @param blend_attachment The blend attachment
  void setBlendAttachment(VkPipelineColorBlendAttachmentState blend_attachment)
  {
    blend_attachment_ = blend_attachment;
  }

  /// @brief Specifies that `buffer` should map to `binding` in the shaders of
  /// this stage.
  void bindBuffer(const BufferResource* buffer, std::uint32_t binding);

  /// @brief Specifies that `shader` should be used during the pipeline of this
  /// stage.
  /// @note Binding two shaders of same type (e.g. two vertex shaders) is
  /// undefined behaviour.
  void usesShader(const wr::Shader& shader);

private:
  bool                                clears_screen_{ false };
  bool                                depth_test_{ false };
  bool                                depth_write_{ false };
  VkPipelineColorBlendAttachmentState blend_attachment_{};
  std::unordered_map<const BufferResource*, std::uint32_t> buffer_bindings_;
  std::vector<VkPipelineShaderStageCreateInfo>             shaders_;
};

class PhysicalResource : public RenderGraphObject {
  friend RenderGraph;

public:
  PhysicalResource(const PhysicalResource&) = delete;
  PhysicalResource(PhysicalResource&&)      = delete;
  ~PhysicalResource() override              = default;

  PhysicalResource& operator=(const PhysicalResource&) = delete;
  PhysicalResource& operator=(PhysicalResource&&)      = delete;

protected:
  const wr::Device& device_;

  explicit PhysicalResource(const wr::Device& device) : device_(device) {}
};

class PhysicalBuffer : public PhysicalResource {
  friend RenderGraph;

public:
  explicit PhysicalBuffer(const wr::Device& device);
  PhysicalBuffer(const PhysicalBuffer&) = delete;
  PhysicalBuffer(PhysicalBuffer&&)      = delete;
  ~PhysicalBuffer()                     = default;

  PhysicalBuffer& operator=(const PhysicalBuffer&) = delete;
  PhysicalBuffer& operator=(PhysicalBuffer&&)      = delete;

private:
  std::unique_ptr<wr::GpuBuffer> buffer_{};
};

class PhysicalImage : public PhysicalResource {
  friend RenderGraph;

public:
  explicit PhysicalImage(const wr::Device& device);
  PhysicalImage(const PhysicalImage&) = delete;
  PhysicalImage(PhysicalImage&&)      = delete;
  ~PhysicalImage() override           = default;

  PhysicalImage& operator=(const PhysicalImage&) = delete;
  PhysicalImage& operator=(PhysicalImage&&)      = delete;

private:
  std::unique_ptr<wr::GpuImage> image_{};
};

class PhysicalBackBuffer : public PhysicalResource {
  friend RenderGraph;

public:
  PhysicalBackBuffer(const wr::Device& device, const wr::Swapchain& swapchain)
    : PhysicalResource(device), m_swapchain(swapchain)
  {
  }
  PhysicalBackBuffer(const PhysicalBackBuffer&) = delete;
  PhysicalBackBuffer(PhysicalBackBuffer&&)      = delete;
  ~PhysicalBackBuffer() override                = default;

  PhysicalBackBuffer& operator=(const PhysicalBackBuffer&) = delete;
  PhysicalBackBuffer& operator=(PhysicalBackBuffer&&)      = delete;

private:
  const wr::Swapchain& m_swapchain;
};

class PhysicalStage : public RenderGraphObject {
  friend RenderGraph;

public:
  explicit PhysicalStage(const wr::Device& device) : device_(device) {}
  PhysicalStage(const PhysicalStage&) = delete;
  PhysicalStage(PhysicalStage&&)      = delete;
  ~PhysicalStage() override           = default;

  PhysicalStage& operator=(const PhysicalStage&) = delete;
  PhysicalStage& operator=(PhysicalStage&&)      = delete;

  /// @brief Retrieve the pipeline layout of this physical stage.
  // TODO: This can be removed once descriptors are properly implemented in the
  // render graph.
  [[nodiscard]] VkPipelineLayout pipelineLayout() const
  {
    return pipeline_->layout();
  }

protected:
  const wr::Device& device_;

private:
  std::unique_ptr<wr::Pipeline> pipeline_;
};

class PhysicalGraphicsStage : public PhysicalStage {
  friend RenderGraph;
  friend wr::Pipeline;

public:
  explicit PhysicalGraphicsStage(const wr::Device& device);
  PhysicalGraphicsStage(const PhysicalGraphicsStage&) = delete;
  PhysicalGraphicsStage(PhysicalGraphicsStage&&)      = delete;
  ~PhysicalGraphicsStage() override                   = default;

  PhysicalGraphicsStage& operator=(const PhysicalGraphicsStage&) = delete;
  PhysicalGraphicsStage& operator=(PhysicalGraphicsStage&&)      = delete;

private:
  std::unique_ptr<wr::RenderPass> render_pass_;
  std::vector<wr::Framebuffer>    framebuffers_;
};

// class ComputePass {};

class RenderGraph {
public:
  explicit RenderGraph(const wr::Device& device, const wr::Swapchain& swapchain)
    : device_(device), swapchain_(swapchain)
  {
  }

  template <typename T, typename... Args> T* add(Args&&... args)
  {
    auto ptr = std::make_unique<T>(std::forward<Args>(args)...);
    if constexpr (std::is_same_v<T, BufferResource>) {
      return static_cast<T*>(
        buffer_resources_.emplace_back(std::move(ptr)).get());
    }
    else if constexpr (std::is_same_v<T, TextureResource>) {
      return static_cast<T*>(
        texture_resources_.emplace_back(std::move(ptr)).get());
    }
    else if constexpr (std::is_base_of_v<RenderStage, T>) {
      return static_cast<T*>(stages_.emplace_back(std::move(ptr)).get());
    }
    else {
      static_assert(!std::is_same_v<T, T>,
                    "T must be a RenderResource or RenderStage");
    }
  }

  void buildRenderPass(const GraphicsStage*, PhysicalGraphicsStage&) const;
  void buildGraphicsPipeline(const GraphicsStage*,
                             PhysicalGraphicsStage&) const;
  // void buildBuffer(const BufferResource&, wr::Buffer&) const;

  void recordCommandBuffer(const RenderStage*       stage,
                           const wr::CommandBuffer& cb,
                           const std::uint32_t      image_index) const;
  void compile(/*const RenderResource* target*/);

  void render(std::uint32_t image_index, const wr::CommandBuffer& cb);

private:
  const wr::Device&               device_;
  const wr::Swapchain&            swapchain_;
  std::shared_ptr<spdlog::logger> log_{ detail::requestLogger("render-graph") };

  std::vector<std::unique_ptr<BufferResource>>  buffer_resources_;
  std::vector<std::unique_ptr<TextureResource>> texture_resources_;
  std::vector<std::unique_ptr<RenderStage>>     stages_;
  // Stage execution order. Each sub-list contains nodes that can be recorded
  // onto the command buffer without a memory barrier in between.
  std::vector<std::vector<RenderStage*>> stage_stack_;
};

template <typename T> [[nodiscard]] T* RenderGraphObject::as()
{
  return dynamic_cast<T*>(this);
}

template <typename T> [[nodiscard]] const T* RenderGraphObject::as() const
{
  return dynamic_cast<const T*>(this);
}

template <typename T> void BufferResource::uploadData(const std::span<T>& data)
{
  data_               = data.data();
  data_size_          = data.size() * (element_size_ = sizeof(T));
  data_upload_needed_ = true;
}

// template <typename T>
// void BufferResource::uploadData(const std::vector<T>& data)
//{
//   uploadData(std::span<T>(data));
// }
} // namespace eldr::vk
