#pragma once
#include <eldr/core/fwd.hpp>
#include <eldr/core/math.hpp>
#include <eldr/render/shape.hpp>

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace eldr {

struct GeoSurface {
  uint32_t start_index;
  uint32_t count;
};

class Mesh : public Shape {
  ELDR_IMPORT_CORE_TYPES();

public:
  Mesh(const std::string& name, std::vector<Point3f>&& positions,
       std::vector<Point2f>&& texcoords, std::vector<Color4f>&& colors,
       std::vector<Vec3f>&& normals, std::vector<GeoSurface>&& surfaces);
  ~Mesh() override = default;

  /// Accessors
  /// @brief Get the name of this mesh
  [[nodiscard]] const std::string&          name() const { return name_; }
  [[nodiscard]] const std::vector<Point3f>& vtxPositions() const
  {
    return vtx_positions_;
  }

  /// @brief Get a vector of vtx texture coordinates for this mest
  [[nodiscard]] const std::vector<Point2f>& vtxTexCoords() const
  {
    return vtx_texcoords_;
  }

  /// @brief Get a vector of vtx normals from this mesh
  [[nodiscard]] const std::vector<Vec3f>& vtxNormals() const
  {
    return vtx_normals_;
  }

  [[nodiscard]] const std::vector<Color4f>& vtxColors() const
  {
    return vtx_colors_;
  }

  [[nodiscard]] static std::optional<std::vector<std::shared_ptr<Mesh>>>
  loadGltfMeshes(std::filesystem::path file_path);

  //  template <typename T>
  [[nodiscard]]
  static std::optional<std::vector<std::shared_ptr<Shape>>>
  loadObj(std::filesystem::path file_path);

protected:
  std::string name_;

  std::vector<Point3f>    vtx_positions_;
  std::vector<Point2f>    vtx_texcoords_;
  std::vector<Color4f>    vtx_colors_;
  std::vector<Vec3f>      vtx_normals_;
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
} // namespace eldr
