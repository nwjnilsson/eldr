#include <render/camera.hpp>
#include <core/vector.hpp>

NAMESPACE_BEGIN(eldr)
class Camera : public PerspectiveCamera<float, Color<float, 3>> {
  using Float = float;
  EL_IMPORT_CORE_TYPES()

public:
  Camera();

  void processInput(const KeyboardMouseInput&);
};
NAMESPACE_END(eldr)
