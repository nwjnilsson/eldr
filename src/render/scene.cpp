#include <eldr/render/scene.hpp>
#include <glm/glm.hpp>
#include <vector>

namespace eldr {

// TODO: remove this struct, just for testing
struct Vertex {
  glm::vec2 pos;
  glm::vec3 color;
};

// TODO: maybe not just an update but also move functions
void Scene::update()
{
  // TODO: set up vertex buffer creation and pass these vertices for rendering
  const std::vector<Vertex> vertices = {
    { { 0.0f, -0.5f }, { 1.0f, 0.0f, 0.0f } },
    { { 0.5f, 0.5f }, { 0.0f, 1.0f, 0.0f } },
    { { -0.5f, 0.5f }, { 0.0f, 0.0f, 1.0f } }
  };
}
} // namespace eldr
