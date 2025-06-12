
#pragma once
NAMESPACE_BEGIN(eldr::render)
template <typename Float> class Sensor : public Endpoint<Float> {
public:
  ~Sensor();

protected:
  Sensor();

private:
  // film
  // resolution
  // shutter time
};
class ProjectiveCamera : public Sensor {
public:
  ~ProjectiveCamera();

protected:
  ProjectiveCamera();

private:
  float near_clip_;
  float far_clip_;
};
NAMESPACE_END(eldr::render)
