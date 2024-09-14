#include <eldr/vulkan/vktools/format.hpp>

std::ostream& operator<<(std::ostream& os, const VkResult& result)
{
  switch (result) {
    case VK_SUCCESS:
      os << "VK_SUCCESS";
      break;
    case VK_NOT_READY:
      os << "VK_NOT_READY";
      break;
    case VK_TIMEOUT:
      os << "VK_TIMEOUT";
      break;
    case VK_EVENT_SET:
      os << "VK_EVENT_SET";
      break;
    case VK_EVENT_RESET:
      os << "VK_EVENT_RESET";
      break;
    case VK_INCOMPLETE:
      os << "VK_INCOMPLETE";
      break;
    case VK_ERROR_OUT_OF_HOST_MEMORY:
      os << "VK_ERROR_OUT_OF_HOST_MEMORY";
      break;
    case VK_ERROR_OUT_OF_DEVICE_MEMORY:
      os << "VK_ERROR_OUT_OF_DEVICE_MEMORY";
      break;
    case VK_ERROR_INITIALIZATION_FAILED:
      os << "VK_ERROR_INITIALIZATION_FAILED";
      break;
    case VK_ERROR_DEVICE_LOST:
      os << "VK_ERROR_DEVICE_LOST";
      break;
    case VK_ERROR_MEMORY_MAP_FAILED:
      os << "VK_ERROR_MEMORY_MAP_FAILED";
      break;
    case VK_ERROR_LAYER_NOT_PRESENT:
      os << "VK_ERROR_LAYER_NOT_PRESENT";
      break;
    case VK_ERROR_EXTENSION_NOT_PRESENT:
      os << "VK_ERROR_EXTENSION_NOT_PRESENT";
      break;
    case VK_ERROR_FEATURE_NOT_PRESENT:
      os << "VK_ERROR_FEATURE_NOT_PRESENT";
      break;
    case VK_ERROR_INCOMPATIBLE_DRIVER:
      os << "VK_ERROR_INCOMPATIBLE_DRIVER";
      break;
    case VK_ERROR_TOO_MANY_OBJECTS:
      os << "VK_ERROR_TOO_MANY_OBJECTS";
      break;
    case VK_ERROR_FORMAT_NOT_SUPPORTED:
      os << "VK_ERROR_FORMAT_NOT_SUPPORTED";
      break;
    case VK_ERROR_FRAGMENTED_POOL:
      os << "VK_ERROR_FRAGMENTED_POOL";
      break;
    case VK_ERROR_UNKNOWN:
      os << "VK_ERROR_UNKNOWN";
      break;
    case VK_ERROR_OUT_OF_POOL_MEMORY:
      os << "VK_ERROR_OUT_OF_POOL_MEMORY";
      break;
    case VK_ERROR_INVALID_EXTERNAL_HANDLE:
      os << "VK_ERROR_INVALID_EXTERNAL_HANDLE";
      break;
    case VK_ERROR_FRAGMENTATION:
      os << "VK_ERROR_FRAGMENTATION";
      break;
    case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS:
      os << "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS";
      break;
    case VK_PIPELINE_COMPILE_REQUIRED:
      os << "VK_PIPELINE_COMPILE_REQUIRED";
      break;
    case VK_ERROR_SURFACE_LOST_KHR:
      os << "VK_ERROR_SURFACE_LOST_KHR";
      break;
    case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
      os << "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
      break;
    case VK_SUBOPTIMAL_KHR:
      os << "VK_SUBOPTIMAL_KHR";
      break;
    case VK_ERROR_OUT_OF_DATE_KHR:
      os << "VK_ERROR_OUT_OF_DATE_KHR";
      break;
    case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
      os << "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
      break;
    case VK_ERROR_VALIDATION_FAILED_EXT:
      os << "VK_ERROR_VALIDATION_FAILED_EXT";
      break;
    case VK_ERROR_INVALID_SHADER_NV:
      os << "VK_ERROR_INVALID_SHADER_NV";
      break;
    case VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR:
      os << "VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR";
      break;
    case VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR:
      os << "VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR";
      break;
    case VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR:
      os << "VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR";
      break;
    case VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR:
      os << "VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR";
      break;
    case VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR:
      os << "VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR";
      break;
    case VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR:
      os << "VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR";
      break;
    case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT:
      os << "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";
      break;
    case VK_ERROR_NOT_PERMITTED_KHR:
      os << "VK_ERROR_NOT_PERMITTED_KHR";
      break;
    case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
      os << "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT";
      break;
    case VK_THREAD_IDLE_KHR:
      os << "VK_THREAD_IDLE_KHR";
      break;
    case VK_THREAD_DONE_KHR:
      os << "VK_THREAD_DONE_KHR";
      break;
    case VK_OPERATION_DEFERRED_KHR:
      os << "VK_OPERATION_DEFERRED_KHR";
      break;
    case VK_OPERATION_NOT_DEFERRED_KHR:
      os << "VK_OPERATION_NOT_DEFERRED_KHR";
      break;
    case VK_ERROR_COMPRESSION_EXHAUSTED_EXT:
      os << "VK_ERROR_COMPRESSION_EXHAUSTED_EXT";
      break;
    case VK_RESULT_MAX_ENUM:
      os << "VK_RESULT_MAX_ENUM";
      break;
    default:
      os << "Unknown result ({" << static_cast<int32_t>(result) << "})";
  }
  return os;
}
