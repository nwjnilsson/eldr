#pragma once

#include <eldr/vulkan/common.hpp>

namespace eldr {
namespace vk {

class Instance {
public:
  Instance(std::vector<const char*>& instance_extensions);
  ~Instance();

  const VkInstance& get() const { return instance_; }
  VkInstance& get() { return instance_; }

private:
  VkInstance instance_;

};
} // namespace vk
} // namespace eldr
