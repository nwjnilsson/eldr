#include <eldr/core/core.hpp>
#include <eldr/render/mesh.hpp>
#include <eldr/render/scene.hpp>
#include <eldr/vulkan/descriptorallocator.hpp> // SceneData
#include <eldr/vulkan/engine.hpp>
#include <eldr/vulkan/material.hpp>
#include <eldr/vulkan/wrappers/image.hpp>
#include <eldr/vulkan/wrappers/sampler.hpp>

#include <eldr/vulkan/sceneresources.hpp>

#include <eldr/ext/fastgltf.hpp>

// Specialize vector types for use with fastgltf. Just simple Vector<float, s>
#define EL_DECLARE_FASTGLTF_ELEMENT_TRAIT_SPEC(Name, Size)                     \
  template <>                                                                  \
  struct ElementTraits<Name<float, Size>>                                      \
    : ElementTraitsBase<Name<float, Size>, AccessorType::Vec##Size, float> {};
NAMESPACE_BEGIN(fastgltf)
EL_DECLARE_FASTGLTF_ELEMENT_TRAIT_SPEC(eldr::core::Point, 2)
EL_DECLARE_FASTGLTF_ELEMENT_TRAIT_SPEC(eldr::core::Point, 3)
EL_DECLARE_FASTGLTF_ELEMENT_TRAIT_SPEC(eldr::core::Vector, 3)
EL_DECLARE_FASTGLTF_ELEMENT_TRAIT_SPEC(eldr::core::Normal, 3)
EL_DECLARE_FASTGLTF_ELEMENT_TRAIT_SPEC(eldr::core::Color, 4)
NAMESPACE_END(fastgltf)

// TODO: use rapidobj
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

using namespace eldr::core;

NAMESPACE_BEGIN(eldr::render)
//------------------------------------------------------------------------------
// Scene node
//------------------------------------------------------------------------------
void SceneNode::refreshTransform(const Transform4f& parent_transform)
{
  world_transform = parent_transform * local_transform;
  for (auto c : children) {
    c->refreshTransform(world_transform);
  }
}

void SceneNode::draw(const RenderObject::Matrix4f& top_matrix,
                     DrawContext&                  ctx) const
{
  for (auto& c : children)
    c->draw(top_matrix, ctx);
}

void SceneNode::map(std::function<void(SceneNode*)> func)
{
  func(this);
  for (auto& c : children)
    c->map(func);
}

//------------------------------------------------------------------------------
// Mesh node
//------------------------------------------------------------------------------
EL_VARIANT void MeshNode<Float, Spectrum>::draw(const Transform4f& top_matrix,
                                                DrawContext&       ctx) const
{
  const Transform4f node_transform{ top_matrix * world_transform };

  for (const auto& s : mesh->surfaces()) {
    const RenderObject obj{
      .index_count = s.count,
      .first_index = s.start_index,
      .material    = s.material.get(),
      //.transform   = node_matrix,
    };
    ctx.opaque_surfaces.push_back(obj);
  }
  // Draw recursively
  SceneNode::draw(top_matrix, ctx);
}

//------------------------------------------------------------------------------
// Scene
//------------------------------------------------------------------------------
EL_VARIANT void Scene<Float, Spectrum>::draw(DrawContext& ctx) const
{
  ctx.opaque_surfaces.clear();
  const Transform4f top_matrix{ 1.f };
  for (auto& n : top_nodes_) {
    n->draw(top_matrix, ctx);
  }
}

EL_INSTANTIATE_CLASS(Scene)
NAMESPACE_END(eldr::render)
