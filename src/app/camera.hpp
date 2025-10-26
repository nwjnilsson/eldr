#include <render/camera.hpp>

NAMESPACE_BEGIN(eldr)
class Camera : public PerspectiveCamera<float, Color<float, 3>> {
  using Base = PerspectiveCamera;
  using Float = float;
  EL_IMPORT_CORE_TYPES()

public:
  Camera();

  void processInput(const KeyboardMouseInput&);
};
NAMESPACE_END(eldr)
