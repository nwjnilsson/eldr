#pragma once
#include <eldr/vulkan/common.hpp>

#include <string>
#include <vector>

namespace eldr::vk::wr {

class Instance {
public:
  Instance() = default;
  Instance(const VkApplicationInfo&, std::vector<const char*>&& extensions);
  ~Instance();

  [[nodiscard]] VkInstance get() const;

private:
  Logger log_{ requestLogger("vulkan-engine") };
  class InstanceImpl;
  std::shared_ptr<InstanceImpl> i_data_;
};

//[[nodiscard]] static bool isExtensionAvailable(const std::string& extension);
//[[nodiscard]] static bool isLayerSupported(const std::string& layer);
} // namespace eldr::vk::wr
