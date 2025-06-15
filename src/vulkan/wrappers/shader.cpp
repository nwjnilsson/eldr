#include <eldr/core/util.hpp>
#include <eldr/vulkan/wrappers/device.hpp>
#include <eldr/vulkan/wrappers/shader.hpp>

#include <fstream>
#include <vector>
NAMESPACE_BEGIN(eldr::vk::wr)

//------------------------------------------------------------------------------
// ShaderModule helper
//------------------------------------------------------------------------------
NAMESPACE_BEGIN()
std::vector<char> loadShader(std::string_view filename)
{
  // TODO: maybe allow more flexibility in loading shaders and don't hardcode
  // path to shader directory

  std::filesystem::path shader_path =
    util::eldrRootDir() / "assets/shaders/" / filename;
  std::ifstream file(shader_path, std::ios::ate | std::ios::binary);

  if (!file.is_open()) {
    Throw("Failed to open file {}!", shader_path.c_str());
  }

  size_t            file_size = (size_t) file.tellg();
  std::vector<char> buffer(file_size);
  file.seekg(0);
  file.read(buffer.data(), file_size);
  file.close();
  return buffer;
}
NAMESPACE_END()

//------------------------------------------------------------------------------
// ShaderModule
//------------------------------------------------------------------------------
EL_VK_IMPL_DEFAULTS(ShaderModule)
EL_VK_IMPL_DESTRUCTOR(ShaderModule)

ShaderModule::ShaderModule(std::string_view      name,
                           const Device&         device,
                           std::string_view      filename,
                           VkShaderStageFlagBits stage)
  : Base(name, device), stage_(stage)
{
  auto                           bytecode = loadShader(filename);
  const VkShaderModuleCreateInfo shader_module_ci{
    .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
    .pNext    = nullptr,
    .flags    = 0,
    .codeSize = bytecode.size(),
    .pCode    = reinterpret_cast<const uint32_t*>(bytecode.data()),
  };
  if (const VkResult result{ vkCreateShaderModule(
        device.logical(), &shader_module_ci, nullptr, &object_) };
      result != VK_SUCCESS)
    Throw("Failed to create shader module from \"{}\" ({})", filename, result);
}

NAMESPACE_END(eldr::vk::wr)
