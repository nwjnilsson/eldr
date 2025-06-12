#pragma once
#include <eldr/core/vector.hpp>

NAMESPACE_BEGIN(eldr::core)

template <typename Point_ /*, Spectrum_*/> struct Ray {

  using Point = Point_;
  using Float = arr::value_t<Point>;
  static constexpr size_t Size{ arr::size_v<Point_> };
  using Vector = eldr::core::Vector<Float, Size>;

  /// Ray origin
  Point o;
  /// Ray direction
  Vector d;
  /// Maximum position on the ray segment
  Float maxt = arr::Largest<Float>;
  /// Time value associated with this ray
  Float time = 0.f;
  /// Wavelength associated with the ray
  // Wavelength wavelengths;

  /// Construct a new ray (o, d) at time 'time'
  Ray(const Point& o, const Vector& d, Float time
      /*const Wavelength& wavelengths*/)
    : o(o), d(d), time(time) /*, wavelengths(wavelengths)*/
  {
  }

  /// Construct a new ray (o, d) with time
  Ray(const Point& o, const Vector& d, const Float& time = 0.f)
    : o(o), d(d), time(time)
  {
  }

  /// Construct a new ray (o, d) with bounds
  Ray(const Point& o, const Vector& d, Float maxt, Float time
      /*const Wavelength& wavelengths*/)
    : o(o), d(d), maxt(maxt), time(time) /*, wavelengths(wavelengths)*/
  {
  }

  /// Copy a ray, but change the maxt value
  Ray(const Ray& r, Float maxt)
    : o(r.o), d(r.d), maxt(maxt), time(r.time) /*, wavelengths(r.wavelengths)*/
  {
  }

  /// Return the position of a point along the ray
  Point operator()(Float t) const
  {
    return o + t * d;
    // dr::fmadd(d, t, o);
  }

  /// Return a ray that points into the opposite direction
  Ray reverse() const
  {
    Ray result;
    result.o    = o;
    result.d    = -d;
    result.maxt = maxt;
    result.time = time;
    // result.wavelengths = wavelengths;
    return result;
  }
};
NAMESPACE_END(eldr::core)
