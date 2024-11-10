#pragma once

#include <eldr/vulkan/common.hpp>

#include <string>

namespace eldr::vk::wr {
class Shader {
public:
  Shader() = delete;
  Shader(const Device&, VkShaderStageFlagBits, const std::string& name,
         const std::string& file_name, const std::string& entry_point = "main");

  Shader(Shader&&) noexcept;
  Shader(const Shader&) = delete;
  ~Shader();

  Shader& operator=(const Shader&) = delete;
  Shader& operator=(Shader&&)      = delete;

  [[nodiscard]] VkShaderStageFlagBits stage() const { return stage_; }
  [[nodiscard]] VkShaderModule        module() const { return shader_module_; }

  [[nodiscard]] const std::string& entryPoint() const { return entry_point_; }

private:
  const Device&         device_;
  std::string           name_{};
  std::string           entry_point_{ "main" };
  VkShaderStageFlagBits stage_{};
  VkShaderModule        shader_module_{ VK_NULL_HANDLE };
};
} // namespace eldr::vk::wr
