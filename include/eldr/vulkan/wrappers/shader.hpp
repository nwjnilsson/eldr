#pragma once
#include <eldr/vulkan/common.hpp>

#include <string>

namespace eldr::vk::wr {
class Shader {
public:
  Shader() = default;
  Shader(const Device&, std::string_view name, std::string_view file_name,
         VkShaderStageFlagBits stage);

  [[nodiscard]] VkShaderStageFlagBits stage() const { return stage_; }
  [[nodiscard]] VkShaderModule        module() const;

  [[nodiscard]] const std::string& entryPoint() const { return entry_point_; }

private:
  std::string name_;
  // Entry point can be used when combining multiple fragment shaders into a
  // single shader module to differentiate between the behaviours, but currently
  // I'm using one fragment shader per shader module (thus the entry point is
  // always "main" for now)
  std::string           entry_point_{ "main" };
  VkShaderStageFlagBits stage_;
  class ShaderImpl;
  std::shared_ptr<ShaderImpl> s_data_;
};
} // namespace eldr::vk::wr
