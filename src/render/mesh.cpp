#include <eldr/core/logger.hpp>
#include <eldr/core/spectrum.hpp>
#include <eldr/render/mesh.hpp>
#include <eldr/vulkan/engine.hpp>

namespace eldr::render {

EL_VARIANT Mesh<Float, Spectrum>::Mesh(std::string_view          name,
                                       std::vector<Point3f>&&    positions,
                                       std::vector<Point2f>&&    texcoords,
                                       std::vector<Color4f>&&    colors,
                                       std::vector<Normal3f>&&   normals,
                                       std::vector<GeoSurface>&& surfaces)
  : Shape<Float, Spectrum>(name, ShapeType::Mesh), vtx_positions_(positions),
    vtx_texcoords_(texcoords), vtx_colors_(colors), vtx_normals_(normals),
    surfaces_(surfaces)

{
}

EL_INSTANTIATE_CLASS(Mesh)
} // namespace eldr::render
