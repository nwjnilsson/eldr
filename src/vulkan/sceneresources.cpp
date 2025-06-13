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
using namespace eldr::core;

NAMESPACE_BEGIN(eldr::vk)

NAMESPACE_BEGIN()
VkFilter extractFilter(fastgltf::Filter filter)
{
  switch (filter) {
    // nearest samplers
    case fastgltf::Filter::Nearest:
    case fastgltf::Filter::NearestMipMapNearest:
    case fastgltf::Filter::NearestMipMapLinear:
      return VK_FILTER_NEAREST;

    // linear samplers
    case fastgltf::Filter::Linear:
    case fastgltf::Filter::LinearMipMapNearest:
    case fastgltf::Filter::LinearMipMapLinear:
    default:
      return VK_FILTER_LINEAR;
  }
}

VkSamplerMipmapMode extractMipmapMode(fastgltf::Filter filter)
{
  switch (filter) {
    case fastgltf::Filter::NearestMipMapNearest:
    case fastgltf::Filter::LinearMipMapNearest:
      return VK_SAMPLER_MIPMAP_MODE_NEAREST;

    case fastgltf::Filter::NearestMipMapLinear:
    case fastgltf::Filter::LinearMipMapLinear:
    default:
      return VK_SAMPLER_MIPMAP_MODE_LINEAR;
  }
}

std::optional<wr::Image> loadImage(const wr::Device& device,
                                   const fg::Asset&  asset,
                                   fg::Image&        image,
                                   const fs::path    texture_dir)
{
  wr::Image newimage;

  std::visit(
    fg::visitor{
      [](auto&) { Log(Error, "Unknown image source data type."); },
      [&](fg::sources::URI& filePath) {
        Assert(filePath.fileByteOffset == 0);
        Assert(filePath.uri.isLocalPath()); // We're only capable of
                                            // loading local files.

        const fs::path path(filePath.uri.path().begin(),
                            filePath.uri.path().end());
        Bitmap         bitmap{ texture_dir / path };
        bitmap.setName(image.name);
        newimage = wr::Image{ device, bitmap };
      },
      [&](fg::sources::Array& array) {
        MemoryStream mstream{ array.bytes.data(), array.bytes.size() };
        Bitmap       bitmap{ &mstream, Bitmap::FileFormat::Auto };
        bitmap.setName(image.name);
        newimage = wr::Image{ device, bitmap };
      },
      [&](fg::sources::BufferView& view) {
        auto& bufferView = asset.bufferViews[view.bufferViewIndex];
        auto& buffer     = asset.buffers[bufferView.bufferIndex];

        std::visit(
          fg::visitor{ // We only care about VectorWithMime here, because we
                       // specify LoadExternalBuffers, meaning all buffers
                       // are already loaded into an array.
                       [](auto&) {
                         Log(Error, "Unexpected buffer view data source type.");
                       },
                       [&](fg::sources::Array& array) {
                         MemoryStream mstream{ array.bytes.data(),
                                               array.bytes.size() };
                         Bitmap bitmap{ &mstream, Bitmap::FileFormat::Auto };
                         bitmap.setName(image.name);
                         newimage = wr::Image{ device, bitmap };
                       } },
          buffer.data);
      },
    },
    image.data);

  // if any of the attempts to load the data failed, we havent written the image
  // so handle is null
  if (newimage.vk() == VK_NULL_HANDLE) {
    return std::nullopt;
  }
  else {
    return newimage;
  }
}

NAMESPACE_END()

SceneResources::SceneResources(const wr::Device&  device,
                               DefaultResources&& default_data)
  : device(device), default_data(std::move(default_data))
{
}

SceneResources::~SceneResources() = default;

void SceneResources::load(fastgltf::Asset& gltf)
{
  //----------------------------------------------------------------------------
  // Load samplers
  //----------------------------------------------------------------------------
  // idea: for each sampler:
  //   pointers.push_back(resource_manager->addSampler(sampler))
  // pointers[samplerIndex] is the sampler to use
  // resource manager adds the sampler if it doesn't already exist
  // should probably return a shared pointer, and that shared pointer should be
  // stored in the material (?). Materials can share samplers and images. One
  // material buffer per scene? Index into buffer using material. Material
  // constants should probably be unique. resourcemanager should have:
  //  - shared pointers to materials
  //  - shared pointers to samplers, images
  //  - shared pointers to material buffers
  for (const fg::Sampler& sampler : gltf.samplers) {
    VkFilter            min_filter{ extractFilter(
      sampler.minFilter.value_or(fg::Filter::Nearest)) };
    VkFilter            mag_filter{ extractFilter(
      sampler.magFilter.value_or(fg::Filter::Nearest)) };
    VkSamplerMipmapMode mipmap_mode{ extractMipmapMode(fg::Filter::Nearest) };
    this->samplers.emplace_back(
      device, min_filter, mag_filter, mipmap_mode, VK_LOD_CLAMP_NONE);
  }

  //----------------------------------------------------------------------------
  // Load textures
  //----------------------------------------------------------------------------
  images.reserve(gltf.images.size() + 1);
  images.push_back(wr::Image::createErrorImage(device));
  for (fg::Image& image : gltf.images) {
    auto img = loadImage(
      device, gltf, image, core::util::eldrRootDir() / "assets/textures");
    if (img.has_value()) {
      images.push_back(std::move(img.value()));
      image_indices.push_back(image_indices.size() + 1);
    }
    else {
      image_indices.push_back(0);
    }
  }

  //----------------------------------------------------------------------------
  // Load materials
  //----------------------------------------------------------------------------
  // Just an estimate of what will be needed
  const std::vector<vk::PoolSizeRatio> sizes{
    { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3 },
    { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3 },
    { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1 }
  };
  // TODO, resize according to gltf and the materials that already exist
  material_descriptors =
    vk::DescriptorAllocator{ static_cast<uint32_t>(gltf.materials.size()),
                             sizes };

  material_buffer = {
    device,
    "Material buffer",
    gltf.materials.size(),
    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
    // VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
  };

  int data_index{ 0 };
  std::vector<GltfMetallicRoughness::MaterialConstants>
    scene_material_constants;

  std::vector<std::shared_ptr<Material>> p_materials;
  for (const fg::Material& mat : gltf.materials) {
    GltfMetallicRoughness::MaterialConstants constants;
    constants.color_factors.x = mat.pbrData.baseColorFactor[0];
    constants.color_factors.y = mat.pbrData.baseColorFactor[1];
    constants.color_factors.z = mat.pbrData.baseColorFactor[2];
    constants.color_factors.w = mat.pbrData.baseColorFactor[3];

    constants.metal_rough_factors.x = mat.pbrData.metallicFactor;
    constants.metal_rough_factors.y = mat.pbrData.roughnessFactor;

    // write material parameters to buffer
    scene_material_constants.push_back(constants);

    MaterialPass pass_type = MaterialPass::MainColor;
    if (mat.alphaMode == fg::AlphaMode::Blend) {
      pass_type = MaterialPass::Transparent;
    }

    GltfMetallicRoughness::MaterialResources material_resources{
      // default the material textures
      .color_texture       = default_data.white_image,
      .color_sampler       = default_data.sampler,
      .metal_rough_texture = default_data.white_image,
      .metal_rough_sampler = default_data.sampler,

      // set the uniform buffer for the material data
      .data_buffer = &material_buffer,
      .data_index  = static_cast<size_t>(data_index),
    };
    // -------------------------------------------------------------------------
    // Color textures
    // -------------------------------------------------------------------------
    if (mat.pbrData.baseColorTexture.has_value()) {
      size_t color_img =
        gltf.textures[mat.pbrData.baseColorTexture.value().textureIndex]
          .imageIndex.value();
      size_t sampler =
        gltf.textures[mat.pbrData.baseColorTexture.value().textureIndex]
          .samplerIndex.value();

      material_resources.color_texture = &images[image_indices[color_img]];
      material_resources.color_sampler = &samplers[sampler];
    }
    // -------------------------------------------------------------------------
    // Metallic roughness
    // -------------------------------------------------------------------------
    if (mat.pbrData.metallicRoughnessTexture.has_value()) {
      size_t img =
        gltf.textures[mat.pbrData.metallicRoughnessTexture.value().textureIndex]
          .imageIndex.value();
      size_t sampler =
        gltf.textures[mat.pbrData.metallicRoughnessTexture.value().textureIndex]
          .samplerIndex.value();
      material_resources.metal_rough_texture = &images[image_indices[img]];
      material_resources.metal_rough_sampler = &samplers[sampler];
    }
    // build material
    auto material = std::make_shared<Material>();
    this->materials.push_back(material);
    // const auto res = materials.insert(std::make_pair(mat.name, material));
    //  if (unlikely(not res.second)) {
    //    Log(Warn, "Scene contains duplicate material name ({}).", mat.name);
    //  }
    material->data = default_data.metal_rough_material->writeMaterial(
      device, pass_type, material_resources, material_descriptors);

    data_index++;
  }
  material_buffer.uploadData(scene_material_constants);
}

NAMESPACE_END(eldr::vk)
