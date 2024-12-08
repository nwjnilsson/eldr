#include <eldr/vulkan/vktools/format.hpp>
#include <eldr/vulkan/wrappers/device.hpp>
#include <eldr/vulkan/wrappers/instance.hpp>
#include <engine_config.hpp>

#define VK_EXT_DEBUG_UTILS_NAME "VK_EXT_debug_utils"

namespace eldr::vk::wr {
//------------------------------------------------------------------------------
// Instance helpers
//------------------------------------------------------------------------------
namespace {
bool isExtensionAvailable(const std::string& extension)
{
  uint32_t                           extensions_count;
  std::vector<VkExtensionProperties> extensions;
  vkEnumerateInstanceExtensionProperties(nullptr, &extensions_count, nullptr);
  extensions.resize(extensions_count);

  if (const auto result = vkEnumerateInstanceExtensionProperties(
        nullptr, &extensions_count, extensions.data());
      result != VK_SUCCESS)
    ThrowVk(result, "vkEnumerateInstanceExtensionProperties(): ");

  if (extensions_count == 0) {
    spdlog::info("No Vulkan instance extensions available!");
    return false;
  }
  for (const VkExtensionProperties& p : extensions)
    if (strcmp(p.extensionName, extension.c_str()) == 0)
      return true;
  return false;
}

bool isLayerSupported(const std::string& layer)
{
  uint32_t layer_count;
  vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

  std::vector<VkLayerProperties> available_layers(layer_count);
  vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

  if (layer_count == 0) {
    spdlog::info("No Vulkan instance layers supported!");
    return false;
  }

  for (const auto& layer_properties : available_layers) {
    if (strcmp(layer.c_str(), layer_properties.layerName) == 0) {
      return true;
    }
  }
  return false;
}
} // namespace

//------------------------------------------------------------------------------
// InstanceImpl
//------------------------------------------------------------------------------
class Instance::InstanceImpl {
public:
  InstanceImpl(const VkInstanceCreateInfo& instance_ci);
  ~InstanceImpl();
  VkInstance instance_{ VK_NULL_HANDLE };
};

Instance::InstanceImpl::InstanceImpl(const VkInstanceCreateInfo& instance_ci)
{
  if (const auto result = vkCreateInstance(&instance_ci, nullptr, &instance_);
      result != VK_SUCCESS)
    ThrowVk(result, "vkCreateInstance(): ");
}

Instance::InstanceImpl::~InstanceImpl()
{
  vkDestroyInstance(instance_, nullptr);
}

//------------------------------------------------------------------------------
// Instance
//------------------------------------------------------------------------------
Instance::Instance(const VkApplicationInfo&   app_info,
                   std::vector<const char*>&& extensions)
{
  log_->trace("Initializing Vulkan instance");
  log_->trace("Application name: {}", app_info.pApplicationName);
  log_->trace("Application version: {}.{}.{}",
              VK_API_VERSION_MAJOR(app_info.applicationVersion),
              VK_API_VERSION_MINOR(app_info.applicationVersion),
              VK_API_VERSION_PATCH(app_info.applicationVersion));
  log_->trace("Engine name: {}", app_info.pEngineName);
  log_->trace("Engine version: {}.{}.{}.{}",
              VK_API_VERSION_VARIANT(app_info.engineVersion),
              VK_API_VERSION_MAJOR(app_info.engineVersion),
              VK_API_VERSION_MINOR(app_info.engineVersion),
              VK_API_VERSION_PATCH(app_info.engineVersion));
  log_->trace("Requested Vulkan API version: {}.{}.{}.{}",
              VK_API_VERSION_VARIANT(required_vk_api_version),
              VK_API_VERSION_MAJOR(required_vk_api_version),
              VK_API_VERSION_MINOR(required_vk_api_version),
              VK_API_VERSION_PATCH(required_vk_api_version));

  std::uint32_t available_api_version = 0;
  if (const auto result = vkEnumerateInstanceVersion(&available_api_version);
      result != VK_SUCCESS) {
    ThrowVk(result, "vkEnumerateInstanceVersion(): ");
  }

  // This code will throw an exception if the required version of Vulkan API is
  // not available on the system
  if (VK_API_VERSION_MAJOR(required_vk_api_version) >
        VK_API_VERSION_MAJOR(available_api_version) ||
      (VK_API_VERSION_MAJOR(required_vk_api_version) ==
         VK_API_VERSION_MAJOR(available_api_version) &&
       VK_API_VERSION_MINOR(required_vk_api_version) >
         VK_API_VERSION_MINOR(available_api_version))) {
    std::string exception_message = fmt::format(
      "Your system does not support the required version of Vulkan API. "
      "Required version: {}.{}.{}.{}. Available "
      "Vulkan API version on this machine: {}.{}.{}. Please update your "
      "graphics drivers!",
      std::to_string(VK_API_VERSION_VARIANT(required_vk_api_version)),
      std::to_string(VK_API_VERSION_MAJOR(required_vk_api_version)),
      std::to_string(VK_API_VERSION_MINOR(required_vk_api_version)),
      std::to_string(VK_API_VERSION_PATCH(required_vk_api_version)),
      std::to_string(VK_API_VERSION_VARIANT(available_api_version)),
      std::to_string(VK_API_VERSION_MAJOR(available_api_version)),
      std::to_string(VK_API_VERSION_MINOR(available_api_version)),
      std::to_string(VK_API_VERSION_PATCH(available_api_version)));
    ThrowVk(VkResult{}, "{}", exception_message);
  }

  VkInstanceCreateInfo instance_ci{};
  instance_ci.sType            = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instance_ci.pApplicationInfo = &app_info;

  if (isExtensionAvailable(
        VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME))
    extensions.push_back(
      VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

#ifdef VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME
  if (isExtensionAvailable(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME)) {
    extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    instance_ci.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
  }
#endif

  // Enabling validation layers (there may be more in the future)
#ifdef ELDR_VULKAN_DEBUG_REPORT
  const std::vector<const char*> validation_layers{
    "VK_LAYER_KHRONOS_validation"
  };
  for (auto& layer : validation_layers) {
    if (!isLayerSupported(layer)) {
      ThrowVk(VkResult{}, "Validation layer {} requested, but is not supported",
              layer);
    }
  }
  instance_ci.enabledLayerCount =
    static_cast<uint32_t>(validation_layers.size());
  instance_ci.ppEnabledLayerNames = validation_layers.data();
  extensions.push_back(VK_EXT_DEBUG_UTILS_NAME);
#endif

  instance_ci.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
  instance_ci.ppEnabledExtensionNames = extensions.data();

  // Create instance
  i_data_ = std::make_shared<InstanceImpl>(instance_ci);
}

VkInstance Instance::get() const { return i_data_->instance_; }
} // namespace eldr::vk::wr
