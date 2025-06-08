#pragma once
#include <eldr/vulkan/wrappers/buffer.hpp>
#include <eldr/vulkan/wrappers/commandbuffer.hpp>
#include <eldr/vulkan/wrappers/descriptorsetlayout.hpp>
#include <eldr/vulkan/wrappers/image.hpp>
#include <eldr/vulkan/wrappers/pipeline.hpp>
#include <eldr/vulkan/wrappers/renderpass.hpp>
#include <eldr/vulkan/wrappers/swapchain.hpp>

#include <functional>
#include <memory>
#include <ranges>
#include <span>
#include <unordered_set>
#include <vector>
#include <vulkan/vulkan_core.h>

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

protected:
  const std::string                 name_;
  std::unique_ptr<PhysicalResource> physical_;
};

// enum class BufferUsage {
//   /// @brief Specifies that the buffer will be used to input index data.
//   IndexBuffer,
//
//   /// @brief Specifies that the buffer will be used to input per vertex data
//   to
//   /// a vertex shader.
//   VertexBuffer,
// };

class BufferResource : public RenderResource {
  friend RenderGraph;

public:
  BufferResource(std::string&& name, VkBufferUsageFlags buffer_usage)
    : RenderResource(name), buffer_usage_(buffer_usage)
  {
  }

  /// @return The usage of this buffer resource
  // BufferUsage usage() const { return usage_; }

  /// @brief Specifies the data that should be uploaded to this buffer at the
  /// start of the next frame.
  /// @param data A span of elements to upload to the buffer
  /// @detail Ensure that `data` stays alive until the next frame has been
  /// drawn.
  template <std::ranges::contiguous_range R> void bindData(const R& data)
  {
    using T = std::ranges::range_value_t<R>;
    std::span<const T> s(data);
    data_ = std::as_bytes(s);
  }

private:
  VkBufferUsageFlags buffer_usage_;
  // Data to upload to the GPU on a call to render().
  std::span<const byte_t> data_;
};

enum class TextureUsage {
  /// @brief Specifies that this texture is an offscreen render target
  Color,
  /// @brief Specifies that this texture is a combined depth/stencil buffer.
  /// @note This may mean that this texture is completely GPU-sided and cannot
  /// be accessed by the CPU in any way.
  DepthStencil,
};

// May want presentable flags in the future, multiple graph outputs/swapchains
// enum class TextureFlags : uint32_t {
//   Presentable = 0x01,
// };

class TextureResource : public RenderResource {
  friend RenderGraph;
  friend GraphicsStage;

public:
  TextureResource(std::string&& name, TextureUsage usage, VkFormat format)
    : RenderResource(name), usage_(usage), format_(format)
  {
    switch (usage_) {
      case TextureUsage::Color:
        clear_value_.color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
        break;
      case TextureUsage::DepthStencil:
        clear_value_.depthStencil = { 1.0f, 0 };
        break;
      default:
        Throw("Clear value for this texture usage has not been defined");
    }
  }

  void setSampleCount(VkSampleCountFlagBits sample_count);

  void setClearValue(VkClearValue clear_value) { clear_value_ = clear_value; }

  void resolvesTo(TextureResource* target);

  // void setFlags(TextureFlags flags);

private:
  const TextureUsage    usage_;
  const VkFormat        format_{ VK_FORMAT_UNDEFINED };
  VkSampleCountFlagBits sample_count_{ VK_SAMPLE_COUNT_1_BIT };
  VkClearValue          clear_value_;
  TextureResource*      resolve_{ nullptr };
  // TextureFlags          flags_;
};

/// @brief A single render stage in the render graph.
/// @note Not to be confused with a vulkan render pass.
class RenderStage : public RenderGraphObject {
  friend RenderGraph;

public:
  RenderStage(const RenderStage&) = delete;
  RenderStage(RenderStage&&)      = delete;
  ~RenderStage() override         = default;

  RenderStage& operator=(const RenderStage&) = delete;
  RenderStage& operator=(RenderStage&&)      = delete;

  [[nodiscard]] const std::string& name() const { return name_; }

  /// @brief Specifies that this stage writes to `resource`.
  RenderStage& writesTo(const RenderResource* resource);

  /// @brief Specifies that this stage reads from `resource`.
  RenderStage& readsFrom(const RenderResource* resource);

  /// @brief Binds a descriptor set layout to this render stage.
  /// @note This function will be removed in the near future, as we are aiming
  /// for users of the API to not have to deal with descriptors at all.
  // TODO: Refactor descriptor management in the render graph
  // void addDescriptorLayout(wr::DescriptorSetLayout& layout)
  // {
  //   descriptor_layouts_.push_back(layout.vk());
  // }

  /// @brief Add a push constant range to this render stage.
  /// @param range The push constant range
  // void addPushConstantRange(VkPushConstantRange range)
  // {
  //   push_constant_ranges_.push_back(range);
  // }

  /// @brief Specifies a function that will be called during command buffer
  /// recording for this stage
  /// @details This function can be used to specify other vulkan commands during
  /// command buffer recording. The most common use for this is for draw
  /// commands.
  RenderStage&
  setOnRecord(std::function<void(const wr::CommandBuffer&)> on_record)
  {
    on_record_ = std::move(on_record);
    return *this;
  }

private:
  bool hasReadDependency(
    const std::vector<std::unique_ptr<RenderStage>>& stages) const;

protected:
  explicit RenderStage(std::string_view name) : name_(name) {}

protected:
  const std::string                         name_;
  std::unique_ptr<PhysicalStage>            physical_;
  std::unordered_set<const RenderResource*> writes_;
  std::unordered_set<const RenderResource*> reads_;

  // std::vector<VkDescriptorSetLayout> descriptor_layouts_;
  // std::vector<VkPushConstantRange>   push_constant_ranges_;
  std::function<void(const wr::CommandBuffer&)> on_record_{ [](auto&) {} };
};

class GraphicsStage : public RenderStage {
  friend RenderGraph;

public:
  explicit GraphicsStage(std::string_view name) : RenderStage(name) {}
  GraphicsStage(const GraphicsStage&) = delete;
  GraphicsStage(GraphicsStage&&)      = delete;
  ~GraphicsStage() override           = default;

  GraphicsStage& operator=(const GraphicsStage&) = delete;
  GraphicsStage& operator=(GraphicsStage&&)      = delete;

  /// @brief Specifies that this stage writes to `resource`.
  GraphicsStage&
  writesTo(const TextureResource* resource,
           VkAttachmentLoadOp     load_op,
           VkAttachmentStoreOp    store_op = VK_ATTACHMENT_STORE_OP_STORE);

  GraphicsStage& readsFrom(const RenderResource* resource);

private:
  using LoadStoreOps = std::pair<VkAttachmentLoadOp, VkAttachmentStoreOp>;
  std::unordered_map<const TextureResource*, LoadStoreOps> load_store_ops_;
  std::unordered_set<const TextureResource*>               resolves_;
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
  explicit PhysicalResource() = default;
};

class PhysicalBuffer : public PhysicalResource {
  friend RenderGraph;

public:
  explicit PhysicalBuffer()             = default;
  PhysicalBuffer(const PhysicalBuffer&) = delete;
  PhysicalBuffer(PhysicalBuffer&&)      = delete;
  ~PhysicalBuffer() override            = default;

  PhysicalBuffer& operator=(const PhysicalBuffer&) = delete;
  PhysicalBuffer& operator=(PhysicalBuffer&&)      = delete;

private:
  wr::Buffer<byte_t> buffer_;
};

class PhysicalImage : public PhysicalResource {
  friend RenderGraph;

public:
  explicit PhysicalImage()            = default;
  PhysicalImage(const PhysicalImage&) = delete;
  PhysicalImage(PhysicalImage&&)      = delete;
  ~PhysicalImage() override           = default;

  PhysicalImage& operator=(const PhysicalImage&) = delete;
  PhysicalImage& operator=(PhysicalImage&&)      = delete;

private:
  wr::Image image_;
};

class PhysicalBackBuffer : public PhysicalImage {
  friend RenderGraph;

public:
  explicit PhysicalBackBuffer()                 = default;
  PhysicalBackBuffer(const PhysicalBackBuffer&) = delete;
  PhysicalBackBuffer(PhysicalBackBuffer&&)      = delete;
  ~PhysicalBackBuffer() override                = default;

  PhysicalBackBuffer& operator=(const PhysicalBackBuffer&) = delete;
  PhysicalBackBuffer& operator=(PhysicalBackBuffer&&)      = delete;
};

class PhysicalStage : public RenderGraphObject {
  friend RenderGraph;

public:
  explicit PhysicalStage()            = default;
  PhysicalStage(const PhysicalStage&) = delete;
  PhysicalStage(PhysicalStage&&)      = delete;
  ~PhysicalStage() override           = default;

  PhysicalStage& operator=(const PhysicalStage&) = delete;
  PhysicalStage& operator=(PhysicalStage&&)      = delete;
};

class PhysicalGraphicsStage : public PhysicalStage {
  friend RenderGraph;

public:
  explicit PhysicalGraphicsStage()                    = default;
  PhysicalGraphicsStage(const PhysicalGraphicsStage&) = delete;
  PhysicalGraphicsStage(PhysicalGraphicsStage&&)      = delete;
  ~PhysicalGraphicsStage() override                   = default;

  PhysicalGraphicsStage& operator=(const PhysicalGraphicsStage&) = delete;
  PhysicalGraphicsStage& operator=(PhysicalGraphicsStage&&)      = delete;

private:
  std::vector<VkRenderingAttachmentInfo>     color_attachments_;
  std::unique_ptr<VkRenderingAttachmentInfo> depth_attachment_;
  // wr::RenderPass               render_pass_;
  // std::vector<wr::Framebuffer> framebuffers_;
};

// class ComputePass {};

class RenderGraph {
public:
  explicit RenderGraph(const wr::Device& device, const wr::Swapchain& swapchain)
    : device_(device), swapchain_(swapchain)
  {
    back_buffer_ = add<TextureResource>(
      "Back buffer", TextureUsage::Color, swapchain_.imageFormat());
  }

  [[nodiscard]] TextureResource*       backBuffer() { return back_buffer_; }
  [[nodiscard]] const TextureResource* backBuffer() const
  {
    return back_buffer_;
  }

  template <typename T, typename... Args> [[nodiscard]] T* add(Args&&... args)
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

  void buildAttachments(const GraphicsStage*, PhysicalGraphicsStage&) const;
  // void buildRenderPass(const GraphicsStage*, PhysicalGraphicsStage&) const;
  // void buildPipelineLayout(const RenderStage*, PhysicalStage&) const;
  // void buildGraphicsPipeline(const GraphicsStage*,
  //                            PhysicalGraphicsStage&) const;

  void recordCommandBuffer(const RenderStage*       stage,
                           const wr::CommandBuffer& cb) const;
  void compile();

  void render(const wr::CommandBuffer& cb, wr::Image& target);

private:
  const wr::Device&    device_;
  const wr::Swapchain& swapchain_;

  TextureResource*                              back_buffer_;
  std::vector<std::unique_ptr<TextureResource>> texture_resources_;
  std::vector<std::unique_ptr<BufferResource>>  buffer_resources_;
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

// EL_DECLARE_ENUM_OPERATORS(TextureFlags)

} // namespace eldr::vk
