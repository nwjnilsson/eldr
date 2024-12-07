#include <eldr/vulkan/wrappers/commandbuffer.hpp>
#include <eldr/vulkan/wrappers/commandpool.hpp>
#include <eldr/vulkan/wrappers/device.hpp>
#include <eldr/vulkan/wrappers/instance.hpp>
#include <eldr/vulkan/wrappers/surface.hpp>

#include <mutex>
#include <set>

namespace eldr::vk::wr {
namespace {
QueueFamilyIndices findQueueFamilies(VkPhysicalDevice physical_device,
                                     VkSurfaceKHR     surface)
{
  QueueFamilyIndices indices;
  uint32_t           count = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &count, nullptr);
  std::vector<VkQueueFamilyProperties> queue_families(count);
  vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &count,
                                           queue_families.data());

  // For now, only the graphics queue is found and set. Perhaps there will
  // be a need for other queues (memory or others) in the future?
  // Note on queue families: it is "very" likely that the graphics queue
  // family will be the same as the present queue family
  for (uint32_t i = 0; i < count; i++) {
    if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      indices.graphics_family = i;
    }
    VkBool32 present_support = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface,
                                         &present_support);
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
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface,
                                            &details.capabilities);
  uint32_t format_count;
  vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count,
                                       nullptr);

  if (format_count != 0) {
    details.formats.resize(format_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface,
                                         &format_count, details.formats.data());
  }

  // Query for present modes
  uint32_t present_mode_count;
  vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface,
                                            &present_mode_count, nullptr);

  if (present_mode_count != 0) {
    details.present_modes.resize(present_mode_count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface,
                                              &present_mode_count,
                                              details.present_modes.data());
  }
  return details;
}

bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface,
                      std::vector<const char*> device_extensions)
{
  // Enumerate physical device extension
  uint32_t                           props_count;
  std::vector<VkExtensionProperties> properties;
  vkEnumerateDeviceExtensionProperties(device, nullptr, &props_count, nullptr);
  properties.resize(props_count);
  vkEnumerateDeviceExtensionProperties(device, nullptr, &props_count,
                                       properties.data());

  // Check extensions
  std::set<std::string> required_extensions(device_extensions.begin(),
                                            device_extensions.end());
  for (const auto& extension : properties) {
    required_extensions.erase(extension.extensionName);
  }
  // Queue families
  QueueFamilyIndices indices = findQueueFamilies(device, surface);

  // Swap chain
  SwapchainSupportDetails swapchain_support =
    getSwapchainSupportDetails(device, surface);

  VkPhysicalDeviceFeatures supported_features;
  vkGetPhysicalDeviceFeatures(device, &supported_features);

  return required_extensions.empty() && indices.isComplete() &&
         !swapchain_support.formats.empty() &&
         !swapchain_support.present_modes.empty() &&
         supported_features.samplerAnisotropy;
}

VkPhysicalDevice
selectPhysicalDevice(VkInstance instance, VkSurfaceKHR surface,
                     std::vector<const char*>& device_extensions)
{
  uint32_t gpu_count;
  VkResult err = vkEnumeratePhysicalDevices(instance, &gpu_count, nullptr);
  if (err != VK_SUCCESS)
    ThrowVk(err, "vkEnumeratePhysicalDevices(): ");

  if (gpu_count == 0)
    ThrowVk(VkResult{}, "No compatible gpu found!");

  std::vector<VkPhysicalDevice> gpus;
  gpus.resize(gpu_count);
  err = vkEnumeratePhysicalDevices(instance, &gpu_count, gpus.data());
  if (err != VK_SUCCESS)
    ThrowVk(err, "vkEnumeratePhysicalDevices(): ");

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
  // Return integrated GPU if no discrete one is present
  return gpus[0];
}
} // namespace

// -----------------------------------------------------------------------------

//------------------------------------------------------------------------------
// DeviceImpl
//------------------------------------------------------------------------------
class Device::DeviceImpl {
public:
  DeviceImpl(VkPhysicalDevice physical, const VkDeviceCreateInfo& device_ci,
             VmaAllocatorCreateInfo& allocator_ci);
  ~DeviceImpl();
  VkPhysicalDevice physical_device_{ VK_NULL_HANDLE };
  VkDevice         device_{ VK_NULL_HANDLE };
  VmaAllocator     allocator_{ VK_NULL_HANDLE };
  // One command pool per thread
  mutable std::vector<CommandPool> command_pools_;
  mutable std::mutex               mutex_;
};

Device::DeviceImpl::DeviceImpl(VkPhysicalDevice          physical,
                               const VkDeviceCreateInfo& device_ci,
                               VmaAllocatorCreateInfo&   allocator_ci)
  : physical_device_(physical)
{
  if (const auto result =
        vkCreateDevice(physical_device_, &device_ci, nullptr, &device_);
      result != VK_SUCCESS)
    ThrowVk(result, "vkCreateDevice(): ");

  allocator_ci.device = device_;
  if (const VkResult result = vmaCreateAllocator(&allocator_ci, &allocator_);
      result != VK_SUCCESS)
    ThrowVk(result, "vmaCreateAllocator(): ");
}

Device::DeviceImpl::~DeviceImpl()
{
  // Ensure that command pools can be cleared properly
  std::lock_guard<std::mutex> guard(mutex_);
  command_pools_.clear();
  vmaDestroyAllocator(allocator_);
  vkDestroyDevice(device_, nullptr);
}

//------------------------------------------------------------------------------
// Device
//------------------------------------------------------------------------------
Device::Device(const Instance& instance, const Surface& surface,
               std::vector<const char*>& required_extensions, Logger logger)
  : log_(logger)
{
  // Select physical device
  VkPhysicalDevice physical_device =
    selectPhysicalDevice(instance.get(), surface.get(), required_extensions);

  vkGetPhysicalDeviceProperties(physical_device, &physical_device_props_);

  queue_family_indices_ = findQueueFamilies(physical_device, surface.get());
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

  // Logical device create info
  const VkDeviceCreateInfo device_ci{
    .sType                 = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
    .pNext                 = {},
    .flags                 = {},
    .queueCreateInfoCount  = static_cast<uint32_t>(queue_create_infos.size()),
    .pQueueCreateInfos     = queue_create_infos.data(),
    .enabledLayerCount     = {},
    .ppEnabledLayerNames   = {},
    .enabledExtensionCount = static_cast<uint32_t>(required_extensions.size()),
    .ppEnabledExtensionNames = required_extensions.data(),
    .pEnabledFeatures        = &device_features,
  };

  // Allocator create info
  VmaVulkanFunctions vma_vulkan_functions{};
  vma_vulkan_functions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
  vma_vulkan_functions.vkGetDeviceProcAddr   = vkGetDeviceProcAddr;
  const VmaAllocatorCreateInfo allocator_ci{
    .flags                          = {},
    .physicalDevice                 = physical_device,
    .device                         = VK_NULL_HANDLE, // set later
    .preferredLargeHeapBlockSize    = {},
    .pAllocationCallbacks           = nullptr,
    .pDeviceMemoryCallbacks         = nullptr,
    .pHeapSizeLimit                 = nullptr,
    .pVulkanFunctions               = &vma_vulkan_functions,
    .instance                       = instance.get(),
    .vulkanApiVersion               = required_vk_api_version,
    .pTypeExternalMemoryHandleTypes = nullptr,
  };

  // Create device
  d_data_ =
    std::make_shared<DeviceImpl>(physical_device, device_ci, allocator_ci);

  // Get queues
  vkGetDeviceQueue(d_data_->device_,
                   queue_family_indices_.present_family.value(), 0, &p_queue_);
  vkGetDeviceQueue(d_data_->device_,
                   queue_family_indices_.graphics_family.value(), 0, &g_queue_);
}

void Device::waitIdle() const { vkDeviceWaitIdle(d_data_->device_); }

VkSampleCountFlagBits Device::findMaxMsaaSampleCount() const
{
  VkSampleCountFlags counts =
    physical_device_props_.limits.framebufferColorSampleCounts &
    physical_device_props_.limits.framebufferDepthSampleCounts;
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
  return getSwapchainSupportDetails(d_data_->physical_device_, surface);
}

VkFormat Device::findSupportedFormat(const std::vector<VkFormat>& candidates,
                                     VkImageTiling                tiling,
                                     VkFormatFeatureFlags features) const
{
  for (VkFormat format : candidates) {
    VkFormatProperties props;
    vkGetPhysicalDeviceFormatProperties(d_data_->physical_device_, format,
                                        &props);
    if (tiling == VK_IMAGE_TILING_LINEAR &&
        (props.linearTilingFeatures & features) == features)
      return format;
    else if (tiling == VK_IMAGE_TILING_OPTIMAL &&
             (props.optimalTilingFeatures & features) == features)
      return format;
  }
  ThrowVk(VkResult{}, "Failed to find supported format!");
}

VkFormat Device::findDepthFormat() const
{
  return findSupportedFormat(
    { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT,
      VK_FORMAT_D24_UNORM_S8_UINT },
    VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

CommandPool& Device::threadGraphicsPool() const
{
  thread_local CommandPool* thread_graphics_pool{ nullptr };
  if (thread_graphics_pool == nullptr) {
    std::scoped_lock locker(d_data_->mutex_);
    thread_graphics_pool = &d_data_->command_pools_.emplace_back(*this);
  }
  return *thread_graphics_pool;
}

const CommandBuffer& Device::requestCommandBuffer()
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
  vkGetPhysicalDeviceMemoryProperties(d_data_->physical_device_, &mem_props);
  for (uint32_t i = 0; i < mem_props.memoryTypeCount; ++i) {
    if (type_filter & (1 << i) &&
        (mem_props.memoryTypes[i].propertyFlags & properties) == properties) {
      return i;
    }
  }
  ThrowVk(VkResult{}, "Failed to find suitable device memory type!");
}

VkPhysicalDevice Device::physical() const { return d_data_->physical_device_; }
VkDevice         Device::logical() const { return d_data_->device_; }
VmaAllocator     Device::allocator() const { return d_data_->allocator_; }

} // namespace eldr::vk::wr
