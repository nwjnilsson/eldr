#include <eldr/vulkan/wrappers/commandbuffer.hpp>
#include <eldr/vulkan/wrappers/commandpool.hpp>
#include <eldr/vulkan/wrappers/device.hpp>
#include <eldr/vulkan/wrappers/instance.hpp>
#include <eldr/vulkan/wrappers/surface.hpp>

#include <deque>
#include <mutex>
#include <set>

NAMESPACE_BEGIN(eldr::vk::wr)
NAMESPACE_BEGIN()
// -----------------------------------------------------------------------------
// Helper functions
// -----------------------------------------------------------------------------
QueueFamilyIndices findQueueFamilies(VkPhysicalDevice physical_device,
                                     VkSurfaceKHR     surface)
{
  QueueFamilyIndices indices;
  uint32_t           count = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &count, nullptr);
  std::vector<VkQueueFamilyProperties> queue_families(count);
  vkGetPhysicalDeviceQueueFamilyProperties(
    physical_device, &count, queue_families.data());

  // For now, only the graphics queue is found and set. Perhaps there will
  // be a need for other queues (memory or others) in the future?
  // Note on queue families: it is "very" likely that the graphics queue
  // family will be the same as the present queue family
  for (uint32_t i = 0; i < count; i++) {
    if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      indices.graphics_family = i;
    }
    VkBool32 present_support = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(
      physical_device, i, surface, &present_support);
    if (present_support)
      indices.present_family = i;
  }
  return indices;
}

SwapchainSupportDetails
getSwapchainSupportDetails(VkPhysicalDevice physical_device,
                           VkSurfaceKHR     surface)
{
  SwapchainSupportDetails details;
  // Query for supported formats
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
    physical_device, surface, &details.capabilities);
  uint32_t format_count;
  vkGetPhysicalDeviceSurfaceFormatsKHR(
    physical_device, surface, &format_count, nullptr);

  if (format_count != 0) {
    details.formats.resize(format_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(
      physical_device, surface, &format_count, details.formats.data());
  }

  // Query for present modes
  uint32_t present_mode_count;
  vkGetPhysicalDeviceSurfacePresentModesKHR(
    physical_device, surface, &present_mode_count, nullptr);

  if (present_mode_count != 0) {
    details.present_modes.resize(present_mode_count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device,
                                              surface,
                                              &present_mode_count,
                                              details.present_modes.data());
  }
  return details;
}

bool isDeviceSuitable(VkPhysicalDevice                device,
                      VkSurfaceKHR                    surface,
                      const std::vector<const char*>& device_extensions)
{
  // Enumerate physical device extension
  uint32_t                           props_count;
  std::vector<VkExtensionProperties> properties;
  vkEnumerateDeviceExtensionProperties(device, nullptr, &props_count, nullptr);
  properties.resize(props_count);
  vkEnumerateDeviceExtensionProperties(
    device, nullptr, &props_count, properties.data());

  // Check extensions
  std::set<std::string> extensions(device_extensions.begin(),
                                   device_extensions.end());
  for (const auto& extension : properties) {
    extensions.erase(extension.extensionName);
  }
  // Queue families
  const QueueFamilyIndices indices{ findQueueFamilies(device, surface) };

  // Swap chain
  SwapchainSupportDetails swapchain_support{ getSwapchainSupportDetails(
    device, surface) };

  constexpr VkImageUsageFlagBits required_usage_bits[]{
    VK_IMAGE_USAGE_TRANSFER_DST_BIT
  };
  bool has_required_usage{ true };
  for (auto required : required_usage_bits) {
    if (not(swapchain_support.capabilities.supportedUsageFlags & required)) {
      has_required_usage = false;
    }
  }

  VkPhysicalDeviceFeatures supported_features;
  vkGetPhysicalDeviceFeatures(device, &supported_features);

  return extensions.empty() && indices.isComplete() &&
         !swapchain_support.formats.empty() &&
         !swapchain_support.present_modes.empty() &&
         supported_features.samplerAnisotropy && has_required_usage;
}

VkPhysicalDevice
selectPhysicalDevice(VkInstance                      instance,
                     VkSurfaceKHR                    surface,
                     const std::vector<const char*>& device_extensions)
{
  uint32_t gpu_count;
  VkResult err{ vkEnumeratePhysicalDevices(instance, &gpu_count, nullptr) };
  if (err != VK_SUCCESS)
    Throw("vkEnumeratePhysicalDevices(): {}", err);

  if (gpu_count == 0)
    Throw("No compatible gpu found!");

  std::vector<VkPhysicalDevice> gpus;
  gpus.resize(gpu_count);
  err = vkEnumeratePhysicalDevices(instance, &gpu_count, gpus.data());
  if (err != VK_SUCCESS)
    Throw("vkEnumeratePhysicalDevices(): {}", err);

  // Find discrete GPU if available
  // It is possible to add additional criteria for what is to be considered
  // a suitable GPU. The Vulkan programming tutorial suggests implementing a
  // scoring system where each usable property of a device would add to its
  // overall score, and when the properties of all devices have been
  // evaluated, the highest scoring device is considered the most suitable
  // one. For now I don't really think it matters. Just use a discrete GPU if
  // it's available.
  for (VkPhysicalDevice& device : gpus) {
    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(device, &props);
    if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
        isDeviceSuitable(device, surface, device_extensions)) {
      return device;
    }
  }
  Throw("Failed to find a suitable physical device.");
}
NAMESPACE_END()
// -----------------------------------------------------------------------------

//------------------------------------------------------------------------------
// DeviceData
//------------------------------------------------------------------------------
struct Device::DeviceData {
  VkPhysicalDeviceProperties physical_device_props;
  VkPhysicalDevice           physical_device{ VK_NULL_HANDLE };
  VmaAllocator               allocator{ VK_NULL_HANDLE };
  // One command pool per thread
  mutable std::mutex              mutex;
  mutable std::deque<CommandPool> command_pools;
};

//------------------------------------------------------------------------------
// Device
//------------------------------------------------------------------------------
EL_VK_IMPL_DEFAULTS(Device)

Device::Device(std::string_view                name,
               const Instance&                 instance,
               const Surface&                  surface,
               const std::vector<const char*>& device_extensions)
  : Base(name), d_(std::make_unique<DeviceData>())

{
  // Select physical device
  d_->physical_device =
    selectPhysicalDevice(instance.vk(), surface.vk(), device_extensions);

  vkGetPhysicalDeviceProperties(d_->physical_device,
                                &d_->physical_device_props);

  queue_family_indices_ = findQueueFamilies(d_->physical_device, surface.vk());
  std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
  std::set<uint32_t>                   unique_queue_families = {
    queue_family_indices_.graphics_family.value(),
    queue_family_indices_.present_family.value()
  };

  float queue_priority = 1.0f;
  for (uint32_t queue_family : unique_queue_families) {
    const VkDeviceQueueCreateInfo queue_create_info{
      .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      .pNext            = {},
      .flags            = {},
      .queueFamilyIndex = queue_family,
      .queueCount       = 1,
      .pQueuePriorities = &queue_priority,
    };
    queue_create_infos.push_back(queue_create_info);
  }

  VkPhysicalDeviceFeatures device_features{};
  device_features.samplerAnisotropy = VK_TRUE;
  device_features.sampleRateShading = VK_TRUE;

  VkPhysicalDeviceSynchronization2Features sync2_features{
    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES,
    .pNext = {},
    .synchronization2 = VK_TRUE
  };

  VkPhysicalDeviceBufferDeviceAddressFeatures buffer_address_features{
    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES,
    .pNext = &sync2_features,
    .bufferDeviceAddress              = VK_TRUE,
    .bufferDeviceAddressCaptureReplay = VK_FALSE,
    .bufferDeviceAddressMultiDevice   = VK_FALSE,
  };

  const VkPhysicalDeviceDynamicRenderingFeatures dynamic_rendering_features{
    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES,
    .pNext = &buffer_address_features,
    .dynamicRendering = VK_TRUE,
  };

  // Logical device create info
  const VkDeviceCreateInfo device_ci{
    .sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
    .pNext                   = &dynamic_rendering_features,
    .flags                   = {},
    .queueCreateInfoCount    = static_cast<uint32_t>(queue_create_infos.size()),
    .pQueueCreateInfos       = queue_create_infos.data(),
    .enabledLayerCount       = {},
    .ppEnabledLayerNames     = {},
    .enabledExtensionCount   = static_cast<uint32_t>(device_extensions.size()),
    .ppEnabledExtensionNames = device_extensions.data(),
    .pEnabledFeatures        = &device_features,
  };

  // Create device
  if (const VkResult result{
        vkCreateDevice(d_->physical_device, &device_ci, nullptr, &object_) };
      result != VK_SUCCESS)
    Throw("vkCreateDevice(): {}", result);

  // Allocator create info
  VmaVulkanFunctions vma_vulkan_functions{};
  vma_vulkan_functions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
  vma_vulkan_functions.vkGetDeviceProcAddr   = vkGetDeviceProcAddr;
  const VmaAllocatorCreateInfo allocator_ci{
    .flags          = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
    .physicalDevice = d_->physical_device,
    .device         = object_,
    .preferredLargeHeapBlockSize    = {},
    .pAllocationCallbacks           = nullptr,
    .pDeviceMemoryCallbacks         = nullptr,
    .pHeapSizeLimit                 = nullptr,
    .pVulkanFunctions               = &vma_vulkan_functions,
    .instance                       = instance.vk(),
    .vulkanApiVersion               = required_vk_api_version,
    .pTypeExternalMemoryHandleTypes = nullptr,
  };

  // Create allocator
  if (const VkResult result = vmaCreateAllocator(&allocator_ci, &d_->allocator);
      result != VK_SUCCESS)
    Throw("vmaCreateAllocator(): {}", result);

  // Get queues
  vkGetDeviceQueue(
    object_, queue_family_indices_.present_family.value(), 0, &p_queue_);
  vkGetDeviceQueue(
    object_, queue_family_indices_.graphics_family.value(), 0, &g_queue_);
}

Device::~Device()
{
  if (vk()) {
    // Ensure that command pools can be cleared properly
    std::lock_guard lock(d_->mutex);
    d_->command_pools.clear();
    vmaDestroyAllocator(d_->allocator);
    vkDestroyDevice(object_, nullptr);
  }
}

void Device::waitIdle() const { vkDeviceWaitIdle(object_); }

VkSampleCountFlagBits Device::findMaxMsaaSampleCount() const
{
  VkSampleCountFlags counts =
    d_->physical_device_props.limits.framebufferColorSampleCounts &
    d_->physical_device_props.limits.framebufferDepthSampleCounts;
  if (counts & VK_SAMPLE_COUNT_64_BIT) {
    return VK_SAMPLE_COUNT_64_BIT;
  }
  if (counts & VK_SAMPLE_COUNT_32_BIT) {
    return VK_SAMPLE_COUNT_32_BIT;
  }
  if (counts & VK_SAMPLE_COUNT_16_BIT) {
    return VK_SAMPLE_COUNT_16_BIT;
  }
  if (counts & VK_SAMPLE_COUNT_8_BIT) {
    return VK_SAMPLE_COUNT_8_BIT;
  }
  if (counts & VK_SAMPLE_COUNT_4_BIT) {
    return VK_SAMPLE_COUNT_4_BIT;
  }
  if (counts & VK_SAMPLE_COUNT_2_BIT) {
    return VK_SAMPLE_COUNT_2_BIT;
  }
  return VK_SAMPLE_COUNT_1_BIT;
}

SwapchainSupportDetails
Device::swapchainSupportDetails(VkSurfaceKHR surface) const
{
  return getSwapchainSupportDetails(d_->physical_device, surface);
}

std::string Device::physicalDeviceName() const
{
  return d_->physical_device_props.deviceName;
}

VkFormat Device::findSupportedFormat(const std::vector<VkFormat>& candidates,
                                     VkImageTiling                tiling,
                                     VkFormatFeatureFlags features) const
{
  for (VkFormat format : candidates) {
    VkFormatProperties props;
    vkGetPhysicalDeviceFormatProperties(d_->physical_device, format, &props);
    if (tiling == VK_IMAGE_TILING_LINEAR &&
        (props.linearTilingFeatures & features) == features)
      return format;
    else if (tiling == VK_IMAGE_TILING_OPTIMAL &&
             (props.optimalTilingFeatures & features) == features)
      return format;
  }
  Throw("Failed to find supported format!");
}

VkFormat Device::findDepthFormat() const
{
  return findSupportedFormat({ VK_FORMAT_D32_SFLOAT,
                               VK_FORMAT_D32_SFLOAT_S8_UINT,
                               VK_FORMAT_D24_UNORM_S8_UINT },
                             VK_IMAGE_TILING_OPTIMAL,
                             VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

CommandPool& Device::threadGraphicsPool() const
{
  thread_local CommandPool* thread_graphics_pool{ nullptr };
  if (thread_graphics_pool == nullptr) {
    std::lock_guard lock(d_->mutex);
    thread_graphics_pool =
      &d_->command_pools.emplace_back("Thread pool", *this);
  }
  return *thread_graphics_pool;
}

const CommandBuffer& Device::requestCommandBuffer() const
{
  return threadGraphicsPool().requestCommandBuffer();
}

void Device::execute(
  const std::function<void(const CommandBuffer& cb)>& cmd_lambda) const
{
  const auto& cb = threadGraphicsPool().requestCommandBuffer();
  cmd_lambda(cb);
  cb.submitAndWait();
}

uint32_t Device::findMemoryType(uint32_t              type_filter,
                                VkMemoryPropertyFlags properties) const
{
  VkPhysicalDeviceMemoryProperties mem_props;
  vkGetPhysicalDeviceMemoryProperties(d_->physical_device, &mem_props);
  for (uint32_t i = 0; i < mem_props.memoryTypeCount; ++i) {
    if (type_filter & (1 << i) &&
        (mem_props.memoryTypes[i].propertyFlags & properties) == properties) {
      return i;
    }
  }
  Throw("Failed to find suitable device memory type!");
}

VkPhysicalDevice Device::physical() const { return d_->physical_device; }
VkDevice         Device::logical() const { return object_; }
VmaAllocator     Device::allocator() const { return d_->allocator; }

NAMESPACE_END(eldr::vk::wr)
