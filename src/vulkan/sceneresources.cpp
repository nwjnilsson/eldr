#include <eldr/core/bitmap.hpp>
#include <eldr/core/mstream.hpp>
#include <eldr/vulkan/sceneresources.hpp>
#include <eldr/vulkan/wrappers/device.hpp>
#include <eldr/vulkan/wrappers/sampler.hpp>

#include <eldr/core/util.hpp>
#include <eldr/vulkan/engine.hpp>

#include <eldr/ext/fastgltf.hpp>

namespace fg = fastgltf;
namespace fs = std::filesystem;
NAMESPACE_BEGIN(eldr::vk)

SceneResources::SceneResources(const wr::Device&  device,
                               DefaultResources&& default_data)
  : device(device), default_data(std::move(default_data))
{
}

SceneResources::~SceneResources() = default;

void SceneResources::load(fastgltf::Asset& gltf) {}

NAMESPACE_END(eldr::vk)
