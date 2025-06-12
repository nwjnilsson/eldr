#pragma once
#include <eldr/render/fwd.hpp>

#include <vector>
NAMESPACE_BEGIN(eldr::app)
class SceneManager {
public:
  SceneManager();
  ~SceneManager();

private:
  std::vector<render::RenderableScene> scenes_;
};
NAMESPACE_END(eldr::app)
