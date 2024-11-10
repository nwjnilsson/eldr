#pragma once

namespace eldr::vk {
struct GpuVertex;

class GraphicsStage;
class PhysicalGraphicsStage;

// RenderGraph types
class RenderGraph;
class BufferResource;
class TextureResource;
enum class TextureUsage;

namespace wr {
class DebugUtilsMessenger;
class Instance;
class Surface;
class Device;
struct QueueFamilyIndices;
class Swapchain;
// class DescriptorSetLayout;
// class DescriptorPool;
class ResourceDescriptor;
class Pipeline;
class CommandPool;
class Sampler;
class RenderPass;
class GpuTexture;
class GpuResource;
class GpuBuffer;
class GpuImage;
class ImageView;
class Framebuffer;
class CommandBuffer;
class Semaphore;
class Fence;
class Shader;
} // namespace wr
} // namespace eldr::vk
