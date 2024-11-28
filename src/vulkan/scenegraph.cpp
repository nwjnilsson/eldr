#include <eldr/vulkan/scenegraph.hpp>

//------------------------------------------------------------------------------
// SceneNode
//------------------------------------------------------------------------------
void SceneNode::refreshTransform(const Mat4f& parent_matrix)
{
  world_transform = parent_matrix * local_transform;
  for (auto c : children) {
    c->refreshTransform(world_transform);
  }
}

void SceneNode::draw(const Mat4f& top_matrix, DrawContext& ctx)
{
  // draw children
  for (auto& c : children) {
    c->draw(top_matrix, ctx);
  }
}

//------------------------------------------------------------------------------
// MeshNode
//------------------------------------------------------------------------------

void MeshNode::draw(const Mat4f& top_matrix, DrawContext& ctx)
{
  glm::mat4 nodeMatrix = top_matrix * world_transform;

  for (auto& s : mesh->surfaces) {
    RenderObject def;
    def.index_count  = s.count;
    def.first_index  = s.startIndex;
    def.index_buffer = mesh->meshBuffers.indexBuffer.buffer;
    def.material     = &s.material->data;

    def.transform             = nodeMatrix;
    def.vertex_buffer_address = mesh->meshBuffers.vertexBufferAddress;

    ctx.opaque_surfaces.push_back(def);
  }

  // recurse down
  SceneNode::draw(top_matrix, ctx);
}
