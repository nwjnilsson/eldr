#pragma once
#include <eldr/render/fwd.hpp>

#include <vector>
namespace eldr::app {
class SceneManager {
public:
  SceneManager();
  ~SceneManager();

private:
  std::vector<render::RenderableScene> scenes_;
};
} // namespace eldr::app
