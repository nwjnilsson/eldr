#pragma once
#include "eldr/core/fwd.hpp"
#include <eldr/vulkan/fwd.hpp>
//------------------------------------------------------------------------------
// vk_mem_alloc.h gives rise to a lot of warnings about unused
// parameters/variables and missing field initializers, hence the suppression
//------------------------------------------------------------------------------
#ifdef __GNUG__
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wunused-parameter"
#  pragma GCC diagnostic ignored "-Wunused-function"
#  pragma GCC diagnostic ignored "-Wunused-variable"
#  pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#  include <vk_mem_alloc.h> // includes vulkan.h
#  pragma GCC diagnostic pop
#endif

#ifdef _MSC_VER
// TODO: try compiling on windows and disable warnings corresponding to the ones
// above
// #  pragma warning(disable:xxxx)
#  include <vk_mem_alloc.h> // includes vulkan.h
// #  pragma warning(default:xxxx)
#endif
// =============================================================================
#include <eldr/core/logger.hpp>
#include <eldr/vulkan/vktools/format.hpp>
namespace eldr::vk {
constexpr uint32_t max_frames_in_flight{ 2 };
constexpr uint32_t required_vk_api_version{ VK_API_VERSION_1_3 };

//------------------------------------------------------------------------------
// Basic enums
//------------------------------------------------------------------------------
enum class MemoryUsage {
  Unknown      = VMA_MEMORY_USAGE_UNKNOWN,
  Auto         = VMA_MEMORY_USAGE_AUTO,
  PreferDevice = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
  PreferHost   = VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
};

enum class LoadOp {
  Load     = VK_ATTACHMENT_LOAD_OP_LOAD,
  Clear    = VK_ATTACHMENT_LOAD_OP_CLEAR,
  DontCare = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
};

enum class StoreOp {
  Store    = VK_ATTACHMENT_STORE_OP_STORE,
  DontCare = VK_ATTACHMENT_STORE_OP_DONT_CARE,
};

//------------------------------------------------------------------------------
// Flags
//------------------------------------------------------------------------------
enum class HostAccess : FlagRep {
  None          = 0x00,
  Sequential    = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
  Random        = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT,
  AllowTransfer = VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT,
  DeviceAddress = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
};
ELDR_DECLARE_ENUM_OPERATORS(HostAccess)

enum class BufferUsage : FlagRep {
  TransferSrc = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
  TransferDst = VK_BUFFER_USAGE_TRANSFER_DST_BIT,
  // VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT,
  // VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT,
  Uniform = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
  Storage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
  Index   = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
  Vertex  = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
  // VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
  ShaderDeviceAddress = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
};
ELDR_DECLARE_ENUM_OPERATORS(BufferUsage)

enum class MemoryProperty : FlagRep {
  DeviceLocal  = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
  HostVisible  = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
  HostCoherent = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
  HostCached   = VK_MEMORY_PROPERTY_HOST_CACHED_BIT,
  Protected    = VK_MEMORY_PROPERTY_PROTECTED_BIT,
};
ELDR_DECLARE_ENUM_OPERATORS(MemoryProperty)

} // namespace eldr::vk
