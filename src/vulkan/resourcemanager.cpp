#include <core/bitmap.hpp>
#include <core/mstream.hpp>
#include <core/util.hpp>
#include <vulkan/material.hpp>
#include <vulkan/resourcemanager.hpp>
#include <vulkan/wrappers/commandbuffer.hpp>
#include <vulkan/wrappers/image.hpp>
#include <vulkan/wrappers/sampler.hpp>

#include <ext/fastgltf.hpp>

NAMESPACE_BEGIN(eldr::vk)

// This is just a guess, the size will change as materials are loaded
constexpr uint32_t initial_pool_size{ 5 };

namespace fg = fastgltf;
namespace fs = std::filesystem;
// -----------------------------------------------------------------------------
// Helpers
// -----------------------------------------------------------------------------
NAMESPACE_BEGIN()
VkFilter extractFilter(fg::Filter filter)
{
  switch (filter) {
    // nearest samplers
    case fg::Filter::Nearest:
    case fg::Filter::NearestMipMapNearest:
    case fg::Filter::NearestMipMapLinear:
      return VK_FILTER_NEAREST;

    // linear samplers
    case fg::Filter::Linear:
    case fg::Filter::LinearMipMapNearest:
    case fg::Filter::LinearMipMapLinear:
    default:
      return VK_FILTER_LINEAR;
  }
}

VkSamplerMipmapMode extractMipmapMode(fg::Filter filter)
{
  switch (filter) {
    case fg::Filter::NearestMipMapNearest:
    case fg::Filter::LinearMipMapNearest:
      return VK_SAMPLER_MIPMAP_MODE_NEAREST;

    case fg::Filter::NearestMipMapLinear:
    case fg::Filter::LinearMipMapLinear:
    default:
      return VK_SAMPLER_MIPMAP_MODE_LINEAR;
  }
}

NAMESPACE_END()

ResourceManager::ResourceManager()                             = default;
ResourceManager::ResourceManager(ResourceManager&&) noexcept   = default;
ResourceManager& ResourceManager::operator=(ResourceManager&&) = default;

struct ResourceManager::Resources {
  // Default data --------------------------------------------------------------
  GltfMetallicRoughness default_metal_rough_material;
  Sampler           default_sampler_linear;
  Image             white_image;
  Image             error_image;
  // ---------------------------------------------------------------------------
  DescriptorAllocator     material_descriptors; // resize for each file loaded
  std::deque<Material>    materials;
  std::deque<Sampler> samplers;
  std::deque<Image>   images;
  vk::Buffer<GltfMetallicRoughness::MaterialConstants> material_buffer;
};

ResourceManager::~ResourceManager() = default;

ResourceManager::ResourceManager(const Device&       device,
                                 GltfMetallicRoughness&& default_material)
  : device_(&device), d_(std::make_unique<Resources>())
{
  d_->default_metal_rough_material = std::move(default_material);
  // Create default white texture
  d_->white_image = Image{ device, Bitmap::createDefaultWhite() };

  // Create default checkerboard error texture
  d_->error_image = Image{ device, Bitmap::createCheckerboard() };

  // Create default linear sampler
  d_->default_sampler_linear = Sampler{ "Default linear",
                                        device,
                                        VK_FILTER_LINEAR,
                                        VK_FILTER_LINEAR,
                                        VK_SAMPLER_MIPMAP_MODE_LINEAR,
                                        d_->white_image.mipLevels() };

  d_->material_descriptors =
    vk::DescriptorAllocator{ initial_pool_size, GltfMetallicRoughness::sizes };
}

const Image& ResourceManager::errorImage() const { return d_->error_image; }

const Sampler& ResourceManager::defaultSampler() const
{
  return d_->default_sampler_linear;
}

std::optional<const Image*> ResourceManager::loadImage(
  const fg::Asset& asset, fg::Image& image, const fs::path& texture_dir)
{
  const std::string name{ image.name };

  Image newimage;
  std::visit(
    fg::visitor{
      [](auto&) { Log(Error, "Unknown image source data type."); },
      [&](fg::sources::URI& file_path) {
        Assert(file_path.fileByteOffset == 0);
        Assert(file_path.uri.isLocalPath()); // We're only capable of
                                             // loading local files.

        const fs::path path(file_path.uri.path().begin(),
                            file_path.uri.path().end());
        Bitmap         bitmap{ texture_dir / path };
        bitmap.setName(name);
        newimage = Image{ *device_, bitmap };
      },
      [&](fg::sources::Array& array) {
        MemoryStream mstream{ array.bytes.data(), array.bytes.size() };
        Bitmap       bitmap{ &mstream, Bitmap::FileFormat::Auto };
        bitmap.setName(name);
        newimage = Image{ *device_, bitmap };
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
                         bitmap.setName(name);
                         newimage = Image{ *device_, bitmap };
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
    d_->images.push_back(std::move(newimage));
    return &d_->images.back();
  }
}

std::vector<const Material*> ResourceManager::load(fg::Asset& gltf)
{
  //----------------------------------------------------------------------------
  // Load samplers
  //----------------------------------------------------------------------------
  std::vector<const Sampler*> samplers;
  for (const fg::Sampler& sampler : gltf.samplers) {
    const std::string   name{ sampler.name };
    VkFilter            min_filter{ extractFilter(
      sampler.minFilter.value_or(fg::Filter::Nearest)) };
    VkFilter            mag_filter{ extractFilter(
      sampler.magFilter.value_or(fg::Filter::Nearest)) };
    VkSamplerMipmapMode mipmap_mode{ extractMipmapMode(fg::Filter::Nearest) };
    d_->samplers.emplace_back(name,
                              *device_,
                              min_filter,
                              mag_filter,
                              mipmap_mode,
                              static_cast<uint32_t>(VK_LOD_CLAMP_NONE));
    samplers.push_back(&d_->samplers.back());
  }

  //----------------------------------------------------------------------------
  // Load textures
  //----------------------------------------------------------------------------
  std::vector<const Image*> images;
  std::vector<size_t>           image_indices;
  images.reserve(gltf.images.size() + 1);
  images.push_back(&d_->error_image);
  for (fg::Image& image : gltf.images) {
    auto img = loadImage(gltf, image, util::eldrRootDir() / "assets/textures");
    if (img.has_value()) {
      images.push_back(img.value());
      image_indices.push_back(image_indices.size() + 1);
    }
    else {
      Log(Warn, "Failed to load image \"{}\"", image.name);
      image_indices.push_back(0);
    }
  }

  //----------------------------------------------------------------------------
  // Load materials
  //----------------------------------------------------------------------------
  size_t new_materials{ gltf.materials.size() };

  if (new_materials > 0) {
    d_->material_descriptors.resize(new_materials + d_->materials.size());
    Buffer<GltfMetallicRoughness::MaterialConstants> new_buffer{
      "Material buffer",
      *device_,
      gltf.materials.size() + new_materials,
      VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
      // VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
    };
    if (unlikely(d_->material_buffer.empty())) {
      // No buffer copy needed
      d_->material_buffer = std::move(new_buffer);
    }
    else {
      // Old data exists, copy it
      const VkBufferCopy2 regions[]{ VkBufferCopy2{
        .sType     = VK_STRUCTURE_TYPE_BUFFER_COPY_2,
        .pNext     = {},
        .srcOffset = {},
        .dstOffset = {},
        .size      = d_->material_buffer.sizeBytes() } };

      device_->execute([&](const CommandBuffer& cb) {
        cb.copyBuffer(new_buffer, d_->material_buffer, regions);
      });

      d_->material_buffer = std::move(new_buffer);
    }
  }

  size_t data_index{ d_->materials.size() };
  std::vector<GltfMetallicRoughness::MaterialConstants> gltf_material_constants;

  std::vector<const Material*> materials;
  size_t                       offset{ d_->materials.size() };
  for (const fg::Material& mat : gltf.materials) {
    // TODO: somehow add materials
    GltfMetallicRoughness::MaterialConstants constants;
    constants.color_factors.x = mat.pbrData.baseColorFactor[0];
    constants.color_factors.y = mat.pbrData.baseColorFactor[1];
    constants.color_factors.z = mat.pbrData.baseColorFactor[2];
    constants.color_factors.w = mat.pbrData.baseColorFactor[3];

    constants.metal_rough_factors.x = mat.pbrData.metallicFactor;
    constants.metal_rough_factors.y = mat.pbrData.roughnessFactor;

    // write material parameters to buffer
    gltf_material_constants.push_back(constants);

    MaterialPass pass_type = MaterialPass::MainColor;
    if (mat.alphaMode == fg::AlphaMode::Blend) {
      pass_type = MaterialPass::Transparent;
    }

    // default the material textures
    GltfMetallicRoughness::Resources material_resources{
      .color_texture       = &d_->white_image,
      .color_sampler       = &d_->default_sampler_linear,
      .metal_rough_texture = &d_->white_image,
      .metal_rough_sampler = &d_->default_sampler_linear,
      .data_buffer         = &d_->material_buffer,
      .data_index          = data_index
    };
    // -------------------------------------------------------------------------
    // Color textures
    // -------------------------------------------------------------------------
    if (mat.pbrData.baseColorTexture.has_value()) {
      size_t texture_idx{
        gltf.textures[mat.pbrData.baseColorTexture.value().textureIndex]
          .imageIndex.value()
      };
      size_t sampler_idx{
        gltf.textures[mat.pbrData.baseColorTexture.value().textureIndex]
          .samplerIndex.value()
      };

      material_resources.color_texture = images[image_indices[texture_idx]];
      material_resources.color_sampler = samplers[sampler_idx];
    }
    else {
      Log(Info,
          "Material \"{}\" is missing a color texture, defaulting to white...",
          mat.name);
    }
    // -------------------------------------------------------------------------
    // Metallic roughness
    // -------------------------------------------------------------------------
    if (mat.pbrData.metallicRoughnessTexture.has_value()) {
      size_t texture_idx =
        gltf.textures[mat.pbrData.metallicRoughnessTexture.value().textureIndex]
          .imageIndex.value();
      size_t sampler_idx =
        gltf.textures[mat.pbrData.metallicRoughnessTexture.value().textureIndex]
          .samplerIndex.value();
      material_resources.metal_rough_texture =
        images[image_indices[texture_idx]];
      material_resources.metal_rough_sampler = samplers[sampler_idx];
    }
    else {
      Log(Info,
          "Material \"{}\" is missing a metallic rough texture, defaulting to "
          "white...",
          mat.name);
    }
    // build material
    Material material;
    // const auto res = materials.insert(std::make_pair(mat.name, material));
    //  if (unlikely(not res.second)) {
    //    Log(Warn, "Scene contains duplicate material name ({}).", mat.name);
    //  }
    material.data = d_->default_metal_rough_material.writeMaterial(
      *device_, pass_type, material_resources, d_->material_descriptors);

    d_->materials.push_back(std::move(material));
    data_index++;
    materials.push_back(&d_->materials.back());
  }
  d_->material_buffer.uploadData(gltf_material_constants, offset);
  return materials;
}

NAMESPACE_END(eldr::vk)
