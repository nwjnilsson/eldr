#pragma once
#include <eldr/core/fwd.hpp>
#include <eldr/render/shape.hpp>

#include <memory>
#include <vector>

namespace eldr::render {

// TODO: decide how to deal with materials. I think an enumeration like this
// could be of use for the Vulkan side of things (selecting pipeline etc) but
// the path tracing side will deal with it differently maybe
enum class MaterialType : uint8_t {
  Metallic,
};

struct GeoSurface {
  uint32_t                  start_index;
  uint32_t                  count;
  std::shared_ptr<Material> material;
};

EL_VARIANT class Mesh final : public Shape<Float, Spectrum> {
  EL_IMPORT_CORE_TYPES()

public:
  Mesh(std::string_view          name,
       std::vector<Point3f>&&    positions,
       std::vector<Point2f>&&    texcoords,
       std::vector<Color4f>&&    colors,
       std::vector<Normal3f>&&   normals,
       std::vector<GeoSurface>&& surfaces);
  ~Mesh() override = default;

  /// Accessors
  [[nodiscard]] const std::vector<Point3f>& vtxPositions() const
  {
    return vtx_positions_;
  }

  /// @brief Get a vector of vtx texture coordinates for this mest
  [[nodiscard]] const std::vector<Point2f>& vtxTexCoords() const
  {
    return vtx_texcoords_;
  }

  /// @brief Get a vector of vtx colors from this mesh
  [[nodiscard]] const std::vector<Color4f>& vtxColors() const
  {
    return vtx_colors_;
  }

  /// @brief Get a vector of vtx normals from this mesh
  [[nodiscard]] const std::vector<Normal3f>& vtxNormals() const
  {
    return vtx_normals_;
  }

  /// @brief Get a vector surface info from this mesh
  [[nodiscard]] const std::vector<GeoSurface>& surfaces() const
  {
    return surfaces_;
  }

protected:
  std::vector<Point3f>    vtx_positions_;
  std::vector<Point2f>    vtx_texcoords_;
  std::vector<Color4f>    vtx_colors_;
  std::vector<Normal3f>   vtx_normals_;
  std::vector<GeoSurface> surfaces_;

  // std::optional<vk::wr::GpuBuffer>
};

// template <>
// std::optional<std::vector<std::shared_ptr<Shape>>>
// Mesh::loadObj<Shape>(vk::VulkanEngine*, std::filesystem::path);
//
// template <>
// std::optional<std::vector<std::shared_ptr<Mesh>>>
// Mesh::loadObj<Mesh>(vk::VulkanEngine*, std::filesystem::path);
} // namespace eldr::render
