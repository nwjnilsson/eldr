#include <app/keyboardmouseinput.hpp>
#include <core/config.hpp>
#include <core/fwd.hpp>
#include <core/logger.hpp>
#include <core/transform.hpp>
#include <render/camera.hpp>

NAMESPACE_BEGIN(eldr)
using Matrix4f = CoreAliases<float>::Matrix4f;

EL_VARIANT ProjectiveCamera<Float, Spectrum>::ProjectiveCamera() {}

EL_VARIANT CoreAliases<Float>::Transform4f
           ProjectiveCamera<Float, Spectrum>::rotation() const
{
  Quat4f pitch_rot{ glm::angleAxis(pitch_, Vector3f{ 1.f, 0.f, 0.f }) };
  Quat4f yaw_rot{ glm::angleAxis(yaw_, Vector3f{ 0.f, -1.f, 0.f }) };
  return Transform4f{ glm::toMat4(yaw_rot) * glm::toMat4(pitch_rot) };
}

EL_VARIANT PerspectiveCamera<Float, Spectrum>::PerspectiveCamera() = default;

EL_VARIANT CoreAliases<Float>::Transform4f
           PerspectiveCamera<Float, Spectrum>::view() const
{
  Transform4f translation{ glm::translate(Matrix4f{ 1.f }, this->position_) };
  return glm::inverse(translation * this->rotation());
  // return glm::lookAt(Point3f{ 2.0f, 2.0f, 2.0f },
  //                    Point3f{ 0.0f, 0.0f, 0.0f },
  //                    Vector3f{ 0.0f, 0.0f, 1.0f });
}
EL_INSTANTIATE_CLASS(PerspectiveCamera)

NAMESPACE_END(eldr)
