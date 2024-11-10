#pragma once
#include <eldr/core/fwd.hpp>
#include <eldr/core/math.hpp>
#include <eldr/render/shape.hpp>

#include <string>
#include <vector>

namespace eldr {
class Mesh : public Shape {
  ELDR_IMPORT_CORE_TYPES();

public:
  Mesh(std::vector<Vec3f>&& positions, std::vector<Vec3f>&& normals,
       std::vector<Vec2f>&& texcoords);

  const std::vector<Vec3f>& vertexPositions() const
  {
    return vertex_positions_;
  }

  const std::vector<Vec3f>& vertexNormals() const { return vertex_normals_; }

  const std::vector<Vec2f>& vertexTexCoords() const
  {
    return vertex_texcoords_;
  }

protected:
  std::string name_;
  // uint32_t    vertex_count_ = 0;
  // uint32_t    face_count_   = 0;

  std::vector<Vec3f> vertex_positions_;
  std::vector<Vec3f> vertex_normals_;
  std::vector<Vec2f> vertex_texcoords_;
};
} // namespace eldr
