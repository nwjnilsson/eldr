#pragma once
#include <eldr/app/fwd.hpp>
#include <eldr/render/sensor.hpp>

#include <eldr/math/glm.hpp>

NAMESPACE_BEGIN(eldr)
class ProjectiveCamera : public Sensor {
public:
  EL_IMPORT_TYPES()
  ~ProjectiveCamera();

  Matrix4f         getRotationMatrix() const;
  virtual Matrix4f getViewMatrix() const = 0;

  void setPosition(Point3f position) { position_ = position; }
  void setViewDirection(float pitch, float yaw)
  {
    pitch_ = pitch;
    yaw_   = yaw;
  }

  Point3f position() const { return position_; }

protected:
  ProjectiveCamera();

protected:
  Point3f position_;
  float   pitch_{ 0.f };
  float   yaw_{ 0.f };

private:
  float near_clip_{ 1.f };
  float far_clip_{ 1000.f };
  float focus_distance_;
};

// Perspective camera type used to render from
class PerspectiveCamera : public ProjectiveCamera {
public:
  PerspectiveCamera();

  Matrix4f getViewMatrix() const override;
};

// Main camera type used to navigate around the scene
class Camera : public PerspectiveCamera {
public:
  Camera();

  void update();
  void processInput(const KeyboardMouseInput&);

private:
  Vector3f velocity_;
};
NAMESPACE_END(eldr)
