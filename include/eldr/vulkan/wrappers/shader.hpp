#pragma once
#include <eldr/vulkan/vulkan.hpp>

#include <string>

NAMESPACE_BEGIN(eldr::vk::wr)
class ShaderModule : public VkDeviceObject<VkShaderModule> {
  using Base = VkDeviceObject<VkShaderModule>;

public:
  EL_VK_IMPORT_DEFAULTS(ShaderModule)
  ShaderModule(std::string_view name,
               const Device&,
               std::string_view      file_name,
               VkShaderStageFlagBits stage);

  [[nodiscard]] VkShaderStageFlagBits stage() const { return stage_; }

  [[nodiscard]] const std::string& entryPoint() const { return entry_point_; }

private:
  // Entry point can be used when combining multiple fragment shaders into a
  // single shader module to differentiate between the behaviours, but currently
  // I'm using one fragment shader per shader module (thus the entry point is
  // always "main" for now)
  std::string           entry_point_{ "main" };
  VkShaderStageFlagBits stage_;
};
NAMESPACE_END(eldr::vk::wr)
