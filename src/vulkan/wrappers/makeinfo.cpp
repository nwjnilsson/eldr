#include <eldr/vulkan/common.hpp>
#include <eldr/vulkan/wrappers/makeinfo.hpp>

namespace eldr::vk::wr {

template <> VkApplicationInfo makeInfo(VkApplicationInfo info)
{
  info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  return info;
}

template <> VkBufferCreateInfo makeInfo(VkBufferCreateInfo info)
{
  info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  return info;
}

template <>
VkCommandBufferAllocateInfo makeInfo(VkCommandBufferAllocateInfo info)
{
  info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  return info;
}

template <> VkCommandBufferBeginInfo makeInfo(VkCommandBufferBeginInfo info)
{
  info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  return info;
}

template <> VkCommandPoolCreateInfo makeInfo(VkCommandPoolCreateInfo info)
{
  info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  return info;
}

template <> VkDebugMarkerMarkerInfoEXT makeInfo(VkDebugMarkerMarkerInfoEXT info)
{
  info.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT;
  return info;
}

template <>
VkDebugMarkerObjectNameInfoEXT makeInfo(VkDebugMarkerObjectNameInfoEXT info)
{
  info.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT;
  return info;
}

template <>
VkDebugMarkerObjectTagInfoEXT makeInfo(VkDebugMarkerObjectTagInfoEXT info)
{
  info.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_TAG_INFO_EXT;
  return info;
}

template <>
VkDebugReportCallbackCreateInfoEXT
makeInfo(VkDebugReportCallbackCreateInfoEXT info)
{
  info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
  return info;
}

template <> VkDescriptorPoolCreateInfo makeInfo(VkDescriptorPoolCreateInfo info)
{
  info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  return info;
}

template <>
VkDescriptorSetAllocateInfo makeInfo(VkDescriptorSetAllocateInfo info)
{
  info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  return info;
}

template <>
VkDescriptorSetLayoutCreateInfo makeInfo(VkDescriptorSetLayoutCreateInfo info)
{
  info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  return info;
}

template <> VkDeviceCreateInfo makeInfo(VkDeviceCreateInfo info)
{
  info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  return info;
}

template <> VkDeviceQueueCreateInfo makeInfo(VkDeviceQueueCreateInfo info)
{
  info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  return info;
}

template <> VkFenceCreateInfo makeInfo(VkFenceCreateInfo info)
{
  info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  return info;
}

template <> VkFramebufferCreateInfo makeInfo(VkFramebufferCreateInfo info)
{
  info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  return info;
}

template <>
VkGraphicsPipelineCreateInfo makeInfo(VkGraphicsPipelineCreateInfo info)
{
  info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  return info;
}

template <> VkImageCreateInfo makeInfo(VkImageCreateInfo info)
{
  info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  return info;
}

template <> VkImageMemoryBarrier makeInfo(VkImageMemoryBarrier info)
{
  info.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  return info;
}

template <> VkImageViewCreateInfo makeInfo(VkImageViewCreateInfo info)
{
  info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  return info;
}

template <> VkInstanceCreateInfo makeInfo(VkInstanceCreateInfo info)
{
  info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  return info;
}

template <> VkMemoryBarrier makeInfo(VkMemoryBarrier info)
{
  info.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
  return info;
}

template <>
VkPipelineColorBlendStateCreateInfo
makeInfo(VkPipelineColorBlendStateCreateInfo info)
{
  info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  return info;
}

template <>
VkPipelineDepthStencilStateCreateInfo
makeInfo(VkPipelineDepthStencilStateCreateInfo info)
{
  info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  return info;
}

template <>
VkPipelineInputAssemblyStateCreateInfo
makeInfo(VkPipelineInputAssemblyStateCreateInfo info)
{
  info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  return info;
}

template <> VkPipelineLayoutCreateInfo makeInfo(VkPipelineLayoutCreateInfo info)
{
  info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  return info;
}

template <>
VkPipelineMultisampleStateCreateInfo
makeInfo(VkPipelineMultisampleStateCreateInfo info)
{
  info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  return info;
}

template <>
VkPipelineRasterizationStateCreateInfo
makeInfo(VkPipelineRasterizationStateCreateInfo info)
{
  info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  return info;
}

template <>
VkPipelineShaderStageCreateInfo makeInfo(VkPipelineShaderStageCreateInfo info)
{
  info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  return info;
}

template <>
VkPipelineVertexInputStateCreateInfo
makeInfo(VkPipelineVertexInputStateCreateInfo info)
{
  info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  return info;
}

template <>
VkPipelineViewportStateCreateInfo
makeInfo(VkPipelineViewportStateCreateInfo info)
{
  info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  return info;
}

template <> VkPresentInfoKHR makeInfo(VkPresentInfoKHR info)
{
  info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  return info;
}

template <> VkRenderPassBeginInfo makeInfo(VkRenderPassBeginInfo info)
{
  info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  return info;
}

template <> VkRenderPassCreateInfo makeInfo(VkRenderPassCreateInfo info)
{
  info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  return info;
}

template <> VkSamplerCreateInfo makeInfo(VkSamplerCreateInfo info)
{
  info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  return info;
}

template <> VkSemaphoreCreateInfo makeInfo(VkSemaphoreCreateInfo info)
{
  info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
  return info;
}

template <> VkShaderModuleCreateInfo makeInfo(VkShaderModuleCreateInfo info)
{
  info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  return info;
}

template <> VkSubmitInfo makeInfo(VkSubmitInfo info)
{
  info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  return info;
}

template <> VkSwapchainCreateInfoKHR makeInfo(VkSwapchainCreateInfoKHR info)
{
  info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  return info;
}

template <> VkWriteDescriptorSet makeInfo(VkWriteDescriptorSet info)
{
  info.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  return info;
}

} // namespace eldr::vk::wr
