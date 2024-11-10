#include <eldr/render/mesh.hpp>

namespace eldr {

Mesh::Mesh(std::vector<Vec3f>&& positions, std::vector<Vec3f>&& normals,
           std::vector<Vec2f>&& texcoords)
  : vertex_positions_(positions), vertex_normals_(normals),
    vertex_texcoords_(texcoords)
{
}

} // namespace eldr
