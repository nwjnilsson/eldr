#pragma once
namespace eldr {
// Material
struct GltfMetallicRoughness;
struct MaterialInstance;
struct Material;
} // namespace eldr

namespace eldr::vk {
class VulkanEngine;
struct SceneResources;
struct GpuVertex;
class ImGuiOverlay;
// EXPERIMENTAL
struct GpuMeshBuffers;

// RenderGraph types
class RenderGraph;
class BufferResource;
class TextureResource;
class GraphicsStage;
class PhysicalStage;
class PhysicalGraphicsStage;
enum class TextureUsage;

class DescriptorWriter;
class DescriptorAllocator;

namespace wr {
class DebugUtilsMessenger;
class Instance;
class Surface;
class Device;
struct QueueFamilyIndices;
class Swapchain;
class DescriptorPool;
class DescriptorSetLayout;
class CommandPool;
class Sampler;
class Pipeline;
// class GraphicsPipeline;
// class ComputePipeline;
class AllocatedBuffer;
template <typename T> class Buffer;
class Image;
class ImageView;
class Framebuffer;
class RenderPass;
class CommandBuffer;
class Semaphore;
class Fence;
class Shader;
} // namespace wr
} // namespace eldr::vk
