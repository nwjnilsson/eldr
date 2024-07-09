#include <eldr/render/mesh.hpp>
#include <eldr/render/renderer.hpp>

namespace eldr {
Renderer::Renderer(int width, int height) : window_(width, height, "Eldr")
{
  vk_wrapper_ = std::make_unique<vk::VulkanWrapper>(window_.getGLFWwindow(),
                                                    window_.getExtensions());
}

Renderer::~Renderer() {}

void Renderer::submitGeometry(const std::vector<Shape*>& shapes)
{
  // Create one big vertex buffer along with index buffer
  // TODO: Shape should contain texture ref probably
  Mesh* mesh = dynamic_cast<Mesh*>(shapes[0]);
  if (mesh != nullptr) {
    vk_wrapper_->submitGeometry(mesh->vertexPositions(),
                                mesh->vertexTexCoords());
  }
}

void Renderer::display() {
  vk_wrapper_->drawFrame();
}

} // Namespace eldr
