#pragma once
#include <eldr/eldr.hpp>
NAMESPACE_BEGIN(eldr)
// Material
struct GltfMetallicRoughness;
struct MaterialInstance;
struct Material;

NAMESPACE_BEGIN(vk)
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

NAMESPACE_BEGIN(wr)
class DebugUtilsMessenger;
template <typename T> class VkObject;
template <typename T> class VkDeviceObject;
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
NAMESPACE_END(wr)
NAMESPACE_END(vk)
NAMESPACE_END(eldr)
