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

private:
  const Device&         device_;
  std::string           name_;
  std::string           entry_point_;
  VkShaderStageFlagBits flags_;
  VkShaderModule        shader_module_{ VK_NULL_HANDLE };
};
} // namespace eldr::vk::wr
