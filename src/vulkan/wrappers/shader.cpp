#include <eldr/vulkan/wrappers/device.hpp>
#include <eldr/vulkan/wrappers/shader.hpp>

#include <fstream>
#include <vector>
namespace eldr::vk::wr {

static std::vector<char> loadShader(const std::string& file_name);

Shader::Shader(const Device& device, VkShaderStageFlagBits stage,
               const std::string& name, const std::string& file_name,
               const std::string& entry_point)
  : device_(device), name_(name), entry_point_(entry_point), stage_(stage)
{
  auto                           bytecode = loadShader(file_name);
  const VkShaderModuleCreateInfo shader_ci{
    .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
    .pNext    = nullptr,
    .flags    = 0,
    .codeSize = bytecode.size(),
    .pCode    = reinterpret_cast<const uint32_t*>(bytecode.data()),
  };

  if (const VkResult result = vkCreateShaderModule(
        device_.logical(), &shader_ci, nullptr, &shader_module_);
      result != VK_SUCCESS) {
    ThrowVk(result,
            "vkCreateShaderModule(): failed to create shader module {}!, ",
            name_);
  }
}

Shader::Shader(Shader&& other) noexcept : device_(other.device_)
{
  stage_         = other.stage_;
  name_          = std::move(other.name_);
  entry_point_   = std::move(other.entry_point_);
  shader_module_ = std::exchange(other.shader_module_, nullptr);
}

Shader::~Shader()
{
  if (shader_module_ != VK_NULL_HANDLE)
    vkDestroyShaderModule(device_.logical(), shader_module_, nullptr);
}

static std::vector<char> loadShader(const std::string& file_name)
{
  // TODO: maybe allow more flexibility in loading shaders and don't hardcode
  // path to shader directory
  const char* env_p = std::getenv("ELDR_DIR");
  if (env_p == nullptr) {
    Throw("loadShader(): Environment not set up correctly");
  }
  std::string shader_path =
    fmt::format("{}/assets/shaders/{}", std::string(env_p), file_name);
  std::ifstream file(shader_path, std::ios::ate | std::ios::binary);

  if (!file.is_open()) {
    Throw("loadShader(): failed to open file {}!", shader_path);
  }

  size_t            file_size = (size_t) file.tellg();
  std::vector<char> buffer(file_size);
  file.seekg(0);
  file.read(buffer.data(), file_size);
  file.close();
  return buffer;
}

} // namespace eldr::vk::wr
