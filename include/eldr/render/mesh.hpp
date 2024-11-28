#pragma once
#include <eldr/core/fwd.hpp>
#include <eldr/core/math.hpp>
#include <eldr/render/shape.hpp>

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
  Mesh(std::vector<Vec3f>&& positions, std::vector<Vec3f>&& normals,
       std::vector<Vec2f>&& texcoords);

  /// Accessors
  /// @brief Get the name of this mesh
  [[nodiscard]] const std::string&        name() const { return name_; }
  [[nodiscard]] const std::vector<Vec3f>& vertexPositions() const
  {
    return vertex_positions_;
  }

  /// @brief Get a vector of vertex normals from this mesh
  [[nodiscard]] const std::vector<Vec3f>& vertexNormals() const
  {
    return vertex_normals_;
  }

  /// @brief Get a vector of vertex texture coordinates for this mest
  [[nodiscard]] const std::vector<Vec2f>& vertexTexCoords() const
  {
    return vertex_texcoords_;
  }

protected:
  std::string             name_;
  std::vector<GeoSurface> surfaces_;

  std::vector<Vec3f> vertex_positions_;
  std::vector<Vec3f> vertex_normals_;
  std::vector<Vec2f> vertex_texcoords_;

  // std::optional<vk::wr::GpuBuffer>
};
} // namespace eldr
