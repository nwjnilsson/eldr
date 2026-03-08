#include <app/keyboardmouseinput.hpp>
#include <core/config.hpp>
#include <core/fwd.hpp>
#include <core/logger.hpp>
#include <core/transform.hpp>
#include <render/camera.hpp>

NAMESPACE_BEGIN(eldr)
namespace em = embr;

EL_VARIANT ProjectiveCamera<Float, Spectrum>::ProjectiveCamera() {}

EL_VARIANT CoreAliases<Float>::AffineTransform4f
           ProjectiveCamera<Float, Spectrum>::rotation() const
{
  QuatF    pitch_rot = em::rotate(Vector3f(1.f, 0.f, 0.f), pitch_);
  QuatF    yaw_rot = em::rotate(Vector3f(0.f, -1.f, 0.f), yaw_);
  Matrix4f rot_mat =
    em::quatToMatrix<Matrix4f>(yaw_rot) * em::quatToMatrix<Matrix4f>(pitch_rot);
  return AffineTransform4f(rot_mat, rot_mat);
}

EL_VARIANT PerspectiveCamera<Float, Spectrum>::PerspectiveCamera() = default;

EL_VARIANT CoreAliases<Float>::ProjectiveTransform4f
           PerspectiveCamera<Float, Spectrum>::view() const
{
  auto translation = AffineTransform4f::translate(Vector3f(this->position_));
  auto combined = translation * this->rotation();
  return combined.inverse();
}
EL_INSTANTIATE_CLASS(PerspectiveCamera)

NAMESPACE_END(eldr)
