#pragma once

#include <eldr/vulkan/common.hpp>

#include <string>
#include <vector>

namespace eldr::vk::wr {

class Instance {
public:
  Instance(const VkApplicationInfo&,
           std::vector<const char*>& instance_extensions);
  ~Instance();

  [[nodiscard]] static bool isExtensionAvailable(const std::string& extension);

  [[nodiscard]] static bool isLayerSupported(const std::string& layer);

  VkInstance get() const { return instance_; }

private:
  VkInstance instance_;
};
} // namespace eldr::vk::wr
