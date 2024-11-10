#pragma once

#include <eldr/core/logger.hpp>
#include <eldr/vulkan/common.hpp>

#include <string>
#include <vector>

namespace eldr::vk::wr {

class Instance {
public:
  Instance(const VkApplicationInfo&, std::vector<const char*>&& extensions);
  ~Instance();

  [[nodiscard]] static bool isExtensionAvailable(const std::string& extension);

  [[nodiscard]] static bool isLayerSupported(const std::string& layer);

  VkInstance get() const { return instance_; }

private:
  VkInstance                      instance_{ VK_NULL_HANDLE };
};
} // namespace eldr::vk::wr
