#pragma once
#include <app/fwd.hpp>
#include <render/sensor.hpp>

#include <math/glm.hpp>

NAMESPACE_BEGIN(eldr)

EL_VARIANT class ICamera {
  EL_IMPORT_CORE_TYPES()
  virtual Transform4f rotation() const = 0;
  virtual float       nearClip() const = 0;
  virtual float       farClip() const  = 0;
};

EL_VARIANT class IProjectiveCamera : public virtual ICamera<Float, Spectrum> {
  virtual CoreAliases<Float>::Transform4f view() const = 0;
};


EL_VARIANT class ProjectiveCamera : public Sensor<Float, Spectrum>,
                                    public virtual ICamera<Float, Spectrum> {
  EL_IMPORT_CORE_TYPES()
public:
  // ~ProjectiveCamera();

  float nearClip() const override { return near_clip_; }
  float farClip() const override { return far_clip_; }

  Transform4f rotation() const override;

  void setViewDirection(float pitch, float yaw)
  {
    pitch_ = pitch;
    yaw_   = yaw;
  }

protected:
  ProjectiveCamera();

public:
  Point3f position_{ 0, 0, 5 };

private:
  float pitch_{ 0.f };
  float yaw_{ 0.f };
  float near_clip_{ 1e-2f };
  float far_clip_{ 1e4f };
  float focus_distance_{ far_clip_ };
};

// Perspective camera type used to render from
EL_VARIANT class PerspectiveCamera
  : public ProjectiveCamera<Float, Spectrum>,
    public virtual IProjectiveCamera<Float, Spectrum> {
  EL_IMPORT_CORE_TYPES()
  using Base = ProjectiveCamera<Float, Spectrum>;

public:
  PerspectiveCamera();

  Transform4f view() const override;
};
NAMESPACE_END(eldr)
