#pragma once
#include <eldr/vulkan/common.hpp>

#include <vector>

namespace eldr::vk::wr {

class Instance {
public:
  Instance() = default;
  Instance(const VkApplicationInfo&, std::vector<const char*>&& extensions);

  [[nodiscard]] VkInstance get() const;

private:
  class InstanceImpl;
  std::shared_ptr<InstanceImpl> i_data_;
};

//[[nodiscard]] static bool isExtensionAvailable(const std::string& extension);
//[[nodiscard]] static bool isLayerSupported(const std::string& layer);
} // namespace eldr::vk::wr
