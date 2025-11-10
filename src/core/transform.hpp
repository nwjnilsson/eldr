#pragma once

#include "embr/matrix.hpp"
#include "fwd.hpp"
#include "vector.hpp"

NAMESPACE_BEGIN(eldr)
template <typename _Point> struct Transform {
  using Float = em::value_t<_Point>;
  EL_IMPORT_CORE_TYPES();
  static constexpr size_t Size{ _Point::Size };

  static Transform lookAt(const Point<Float, 3>&  origin,
                          const Point<Float, 3>&  target,
                          const Vector<Float, 3>& up)
  {

    const Vector3f f(normalize(target - origin));
    const Vector3f s(normalize(cross(up, f)));
    const Vector3f u(cross(f, s));

    em::Matrix<Float, 4> result{ 1 };
    result[0][0] = s.x;
    result[1][0] = s.y;
    result[2][0] = s.z;
    result[0][1] = u.x;
    result[1][1] = u.y;
    result[2][1] = u.z;
    result[0][2] = f.x;
    result[1][2] = f.y;
    result[2][2] = f.z;
    result[3][0] = -dot(s, origin);
    result[3][1] = -dot(u, origin);
    result[3][2] = -dot(f, origin);
    return result;
  }

private:
  em::Matrix<Float, Size> transform;
};
NAMESPACE_END(eldr)
