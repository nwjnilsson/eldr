#include <eldr/core/bitmap.hpp>
#include <eldr/core/mstream.hpp>
#include <eldr/core/util.hpp>
#include <eldr/vulkan/resourcemanager.hpp>
#include <eldr/vulkan/wrappers/image.hpp>
#include <eldr/vulkan/wrappers/sampler.hpp>

#include <eldr/ext/fastgltf.hpp>

NAMESPACE_BEGIN(eldr::vk)

// This is just a guess, the size will change as materials are loaded
constexpr uint32_t initial_pool_size{ 5 };

using namespace eldr::vk::wr;
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

ResourceManager::ResourceManager(ResourceManager&&) noexcept = default;
ResourceManager::~ResourceManager()                          = default;
ResourceManager&
ResourceManager::operator=(ResourceManager&&) noexcept = default;

// Materials use samplers and images
// Meshes use materials
struct ResourceManager::Resources {
  // Default data --------------------------------------------------------------
  GltfMetallicRoughness        default_metal_rough_material;
  std::shared_ptr<wr::Sampler> default_sampler_linear;
  std::shared_ptr<wr::Image>   white_image;
  std::shared_ptr<wr::Image>   error_image;
  // ---------------------------------------------------------------------------
  DescriptorAllocator material_descriptors; // resize for each file loaded
  std::unordered_map<std::string, std::shared_ptr<Material>>    materials;
  std::unordered_map<std::string, std::shared_ptr<wr::Sampler>> samplers;
  std::unordered_map<std::string, std::shared_ptr<wr::Image>>   images;
  vk::wr::Buffer<GltfMetallicRoughness::MaterialConstants>      material_buffer;
};

ResourceManager::ResourceManager(const wr::Device&       device,
                                 GltfMetallicRoughness&& default_material)
  : device_(&device), d_(std::make_unique<Resources>())
{
  d_->default_metal_rough_material = std::move(default_material);
  // Create default white texture
  d_->white_image =
    std::make_shared<Image>(device, Bitmap::createDefaultWhite());

  // Create default checkerboard error texture
  d_->error_image =
    std::make_shared<Image>(device, Bitmap::createCheckerboard());

  // Create default linear sampler
  d_->default_sampler_linear =
    std::make_shared<Sampler>("Default linear",
                              device,
                              VK_FILTER_LINEAR,
                              VK_FILTER_LINEAR,
                              VK_SAMPLER_MIPMAP_MODE_LINEAR,
                              d_->white_image->mipLevels());

  d_->material_descriptors =
    vk::DescriptorAllocator{ initial_pool_size, GltfMetallicRoughness::Sizes };
}

std::shared_ptr<Image> const ResourceManager::errorImage() const
{
  return d_->error_image;
}
std::shared_ptr<Sampler> const ResourceManager::defaultSampler() const
{
  return d_->default_sampler_linear;
}

std::optional<std::shared_ptr<wr::Image>> ResourceManager::loadImage(
  const fg::Asset& asset, fg::Image& image, const fs::path& texture_dir)
{
  std::string const name{ image.name };
  if (d_->images.contains(name)) {
    return d_->images[name];
  }

  std::shared_ptr<wr::Image> newimage;
  std::visit(fg::visitor{
               [](auto&) { Log(Error, "Unknown image source data type."); },
               [&](fg::sources::URI& filePath) {
                 Assert(filePath.fileByteOffset == 0);
                 Assert(filePath.uri.isLocalPath()); // We're only capable of
                                                     // loading local files.

                 const fs::path path(filePath.uri.path().begin(),
                                     filePath.uri.path().end());
                 Bitmap         bitmap{ texture_dir / path };
                 bitmap.setName(image.name);
                 newimage = std::make_shared<wr::Image>(device_, bitmap);
               },
               [&](fg::sources::Array& array) {
                 MemoryStream mstream{ array.bytes.data(), array.bytes.size() };
                 Bitmap       bitmap{ &mstream, Bitmap::FileFormat::Auto };
                 bitmap.setName(image.name);
                 newimage = std::make_shared<wr::Image>(device_, bitmap);
               },
               [&](fg::sources::BufferView& view) {
                 auto& bufferView = asset.bufferViews[view.bufferViewIndex];
                 auto& buffer     = asset.buffers[bufferView.bufferIndex];

                 std::visit(
                   fg::visitor{
                     // We only care about VectorWithMime here, because we
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
                       newimage = std::make_shared<wr::Image>(device_, bitmap);
                     } },
                   buffer.data);
               },
             },
             image.data);

  // if any of the attempts to load the data failed, we havent written the image
  // so handle is null
  if (newimage->vk() == VK_NULL_HANDLE) {
    return std::nullopt;
  }
  else {
    d_->images[name] = std::move(newimage);
    return d_->images[name];
  }
}

// TODO return a vector of materials that can be indexed as the gltf would like
void ResourceManager::load(fg::Asset& gltf)
{
  //----------------------------------------------------------------------------
  // Load samplers
  //----------------------------------------------------------------------------
  std::vector<std::shared_ptr<Sampler>> samplers;
  for (fg::Sampler const& sampler : gltf.samplers) {
    std::string name{ sampler.name };
    if (d_->samplers.contains(name)) {
      samplers.push_back(d_->samplers[name]);
    }
    else {
      VkFilter            min_filter{ extractFilter(
        sampler.minFilter.value_or(fg::Filter::Nearest)) };
      VkFilter            mag_filter{ extractFilter(
        sampler.magFilter.value_or(fg::Filter::Nearest)) };
      VkSamplerMipmapMode mipmap_mode{ extractMipmapMode(fg::Filter::Nearest) };
      d_->samplers[name] = std::make_shared<Sampler>(
        name, device_, min_filter, mag_filter, mipmap_mode, VK_LOD_CLAMP_NONE);
      samplers.push_back(d_->samplers[name]);
    }
  }

  //----------------------------------------------------------------------------
  // Load textures
  //----------------------------------------------------------------------------
  std::vector<std::shared_ptr<wr::Image>> images;
  std::vector<size_t>                     image_indices;
  images.reserve(gltf.images.size() + 1);
  images.push_back(d_->error_image);
  for (fg::Image& image : gltf.images) {
    auto img = loadImage(gltf, image, util::eldrRootDir() / "assets/textures");
    if (img.has_value()) {
      images.push_back(std::move(img.value()));
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
  size_t new_materials{ 0 };
  for (const fg::Material& mat : gltf.materials) {
    std::string name{ mat.name };
    if (d_->materials.contains(name)) {
      new_materials++;
    }
  }
  d_->material_descriptors.resize(new_materials + d_->materials.size());

  // TODO: resize buffer and upload data to specific region of buffer, i.e
  // data_index + existing materials.size()
  d_->material_buffer = {
    device_,
    "Material buffer",
    gltf.materials.size(),
    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
    // VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
  };

  size_t data_index{ d_->materials.size() };
  std::vector<GltfMetallicRoughness::MaterialConstants> gltf_material_constants;

  std::vector<std::shared_ptr<Material>> materials;
  for (const fg::Material& mat : gltf.materials) {
    std::string name{ mat.name };
    if (d_->materials.contains(name)) {
      materials.push_back(d_->materials[name]);
      continue;
    }
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

    GltfMetallicRoughness::Resources material_resources{
      // default the material textures
      .color_texture       = d_->white_image,
      .color_sampler       = d_->default_sampler_linear,
      .metal_rough_texture = d_->white_image,
      .metal_rough_sampler = d_->default_sampler_linear,

      // set the uniform buffer for the material data
      .data_buffer = &d_->material_buffer,
      .data_index  = data_index,
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

      material_resources.color_texture = images[image_indices[color_img]];
      material_resources.color_sampler = samplers[sampler];
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
      size_t img =
        gltf.textures[mat.pbrData.metallicRoughnessTexture.value().textureIndex]
          .imageIndex.value();
      size_t sampler =
        gltf.textures[mat.pbrData.metallicRoughnessTexture.value().textureIndex]
          .samplerIndex.value();
      material_resources.metal_rough_texture = images[image_indices[img]];
      material_resources.metal_rough_sampler = samplers[sampler];
    }
    else {
      Log(Info,
          "Material \"{}\" is missing a metallic rough texture, defaulting to "
          "white...",
          mat.name);
    }
    // build material
    auto material = std::make_shared<Material>();
    // const auto res = materials.insert(std::make_pair(mat.name, material));
    //  if (unlikely(not res.second)) {
    //    Log(Warn, "Scene contains duplicate material name ({}).", mat.name);
    //  }
    material->data = d_->default_metal_rough_material.writeMaterial(
      *device_, pass_type, material_resources, d_->material_descriptors);

    d_->materials[name] = std::move(material);
    data_index++;
  }
  material_buffer.uploadData(gltf_material_constants);
}

NAMESPACE_END(eldr::vk)
