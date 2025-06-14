#pragma once
#include <eldr/vulkan/vulkan.hpp>

#include <vector>

namespace eldr::vk::wr {

class Instance {
public:
  Instance();
  Instance(const VkApplicationInfo&, std::vector<const char*>&& extensions);
  ~Instance();

  Instance& operator=(Instance&&);

  [[nodiscard]] VkInstance vk() const;

private:
  class InstanceImpl;
  std::unique_ptr<InstanceImpl> d_;
};

//[[nodiscard]] static bool isExtensionAvailable(const std::string& extension);
//[[nodiscard]] static bool isLayerSupported(const std::string& layer);
} // namespace eldr::vk::wr
