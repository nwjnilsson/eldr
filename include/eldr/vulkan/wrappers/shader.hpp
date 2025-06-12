#pragma once
#include <eldr/vulkan/vulkan.hpp>

#include <string>

NAMESPACE_BEGIN(eldr::vk::wr)
class Shader {
public:
  Shader();
  Shader(const Device&,
         std::string_view      name,
         std::string_view      file_name,
         VkShaderStageFlagBits stage);
  Shader(Shader&&) noexcept;
  ~Shader();

  Shader& operator=(Shader&&);

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
  std::unique_ptr<ShaderImpl> d_;
};
NAMESPACE_END(eldr::vk::wr)
