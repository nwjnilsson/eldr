#pragma once

namespace eldr::vk {
struct GpuVertex;
class ImGuiOverlay;

// RenderGraph types
class RenderGraph;
class BufferResource;
class TextureResource;
class GraphicsStage;
class PhysicalGraphicsStage;
enum class TextureUsage;

namespace wr {
class DebugUtilsMessenger;
class Instance;
class Surface;
class Device;
struct QueueFamilyIndices;
class Swapchain;
class ResourceDescriptor;
class CommandPool;
class Sampler;
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
