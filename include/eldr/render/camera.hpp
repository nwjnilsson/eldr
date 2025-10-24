#pragma once
#include <eldr/app/fwd.hpp>
#include <eldr/render/sensor.hpp>

#include <eldr/math/glm.hpp>

NAMESPACE_BEGIN(eldr)
EL_VARIANT class ProjectiveCamera : public Sensor<Float, Spectrum> {
  EL_IMPORT_TYPES()
public:
  // ~ProjectiveCamera();

  float nearClip() const { return near_clip_; }
  float farClip() const { return far_clip_; }

  Matrix4f         getRotationMatrix() const;
  virtual Matrix4f getViewMatrix() const = 0;

  void setPosition(Point3f position) { position_ = position; }
  void setViewDirection(float pitch, float yaw)
  {
    pitch_ = pitch;
    yaw_   = yaw;
  }

  Point3f getPosition() const { return position_; }

protected:
  ProjectiveCamera();

protected:
  Point3f position_{ 0, 0, 5 };
  float   pitch_{ 0.f };
  float   yaw_{ 0.f };

private:
  float near_clip_{ 1e-2f };
  float far_clip_{ 1e4f };
  float focus_distance_{ far_clip_ };
};

// Perspective camera type used to render from
EL_VARIANT class PerspectiveCamera : public ProjectiveCamera<Float, Spectrum> {
  EL_IMPORT_CORE_TYPES()
  using Base = ProjectiveCamera<Float, Spectrum>;

public:
  PerspectiveCamera();

  Matrix4f getViewMatrix() const override;
};

// Main camera type used to navigate around the scene, maybe move this to vulkan
// engine
class Camera : public PerspectiveCamera<float, Color<float, 3>> {
  using Base = PerspectiveCamera;
  EL_IMPORT_CORE_TYPES_SCALAR()

public:
  Camera();

  void processInput(const KeyboardMouseInput&);
};
NAMESPACE_END(eldr)
