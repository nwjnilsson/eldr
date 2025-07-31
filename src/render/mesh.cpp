#include <eldr/core/core.hpp>
#include <eldr/render/mesh.hpp>
#include <eldr/vulkan/engine.hpp>

NAMESPACE_BEGIN(eldr)

Mesh::Mesh(std::string_view          name,
           std::vector<Point3f>&&    positions,
           std::vector<Point2f>&&    texcoords,
           std::vector<Color4f>&&    colors,
           std::vector<Normal3f>&&   normals,
           std::vector<GeoSurface>&& surfaces)
  : Shape(name, ShapeType::Mesh), positions_(positions), texcoords_(texcoords),
    colors_(colors), normals_(normals), surfaces_(surfaces)
{
}

Mesh::~Mesh() = default;

// EL_INSTANTIATE_CLASS(Mesh)
NAMESPACE_END(eldr)
