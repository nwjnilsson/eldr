#include <eldr/vulkan/wrappers/device.hpp>
#include <eldr/vulkan/wrappers/shader.hpp>

#include <fstream>
#include <vector>
namespace eldr::vk::wr {

//------------------------------------------------------------------------------
// Shader helper
//------------------------------------------------------------------------------
namespace {
std::vector<char> loadShader(std::string_view filename)
{
  // TODO: maybe allow more flexibility in loading shaders and don't hardcode
  // path to shader directory
  const char* env_p = std::getenv("ELDR_DIR");
  if (env_p == nullptr) {
    Throw("loadShader(): Environment not set up correctly");
  }
  std::string shader_path =
    fmt::format("{}/assets/shaders/{}", std::string(env_p), filename);
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
} // namespace
//------------------------------------------------------------------------------
// ShaderImpl
//------------------------------------------------------------------------------
class Shader::ShaderImpl {
public:
  ShaderImpl(const Device&                   device,
             std::string_view                name,
             const VkShaderModuleCreateInfo& shader_module_ci);
  ~ShaderImpl();
  const Device&  device_;
  VkShaderModule shader_module_{ VK_NULL_HANDLE };
};

Shader::ShaderImpl::ShaderImpl(const Device&                   device,
                               std::string_view                filename,
                               const VkShaderModuleCreateInfo& shader_module_ci)
  : device_(device)
{
  if (const VkResult result{ vkCreateShaderModule(
        device_.logical(), &shader_module_ci, nullptr, &shader_module_) };
      result != VK_SUCCESS)
    Throw("Failed to create shader module '{}'! ({})", filename, result);
}

Shader::ShaderImpl::~ShaderImpl()
{
  vkDestroyShaderModule(device_.logical(), shader_module_, nullptr);
}

//------------------------------------------------------------------------------
// Shader
//------------------------------------------------------------------------------
Shader::Shader()                    = default;
Shader::Shader(Shader&&) noexcept   = default;
Shader::~Shader()                   = default;
Shader& Shader::operator=(Shader&&) = default;

Shader::Shader(const Device&         device,
               std::string_view      name,
               std::string_view      filename,
               VkShaderStageFlagBits stage)
  : name_(name), stage_(stage)
{
  auto                           bytecode = loadShader(filename);
  const VkShaderModuleCreateInfo shader_module_ci{
    .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
    .pNext    = nullptr,
    .flags    = 0,
    .codeSize = bytecode.size(),
    .pCode    = reinterpret_cast<const uint32_t*>(bytecode.data()),
  };
  d_ = std::make_unique<ShaderImpl>(device, filename, shader_module_ci);
}

VkShaderModule Shader::module() const { return d_->shader_module_; }

} // namespace eldr::vk::wr
