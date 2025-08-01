#include <eldr/app/keyboardmouseinput.hpp>
#include <eldr/core/logger.hpp>
#include <eldr/render/camera.hpp>

#include <GLFW/glfw3.h>

NAMESPACE_BEGIN(eldr)
using Matrix4f = ProjectiveCamera::Matrix4f;

ProjectiveCamera::ProjectiveCamera() {}

Matrix4f ProjectiveCamera::getRotationMatrix() const
{
  Quat4f pitch_rot{ glm::angleAxis(pitch_, Vector3f{ 1.f, 0.f, 0.f }) };
  Quat4f yaw_rot{ glm::angleAxis(yaw_, Vector3f{ 0.f, -1.f, 0.f }) };
  return glm::toMat4(yaw_rot) * glm::toMat4(pitch_rot);
}

PerspectiveCamera::PerspectiveCamera() = default;

Matrix4f PerspectiveCamera::getViewMatrix() const
{
  Matrix4f translation{ glm::translate(Matrix4f{ 1.f }, position_) };
  return glm::inverse(translation * getRotationMatrix());
  // return glm::lookAt(Point3f{ 2.0f, 2.0f, 2.0f },
  //                    Point3f{ 0.0f, 0.0f, 0.0f },
  //                    Vector3f{ 0.0f, 0.0f, 1.0f });
}

Camera::Camera() = default;

void Camera::processInput(const KeyboardMouseInput& input_data)
{
  // X
  Vector3f velocity{ 0.f };
  if (input_data.isKeyPressed(GLFW_KEY_A)) {
    velocity.x = -0.5f;
  }
  else if (input_data.isKeyPressed(GLFW_KEY_D)) {
    velocity.x = 0.5f;
  }
  else {
    velocity.x = 0;
  }
  // Y
  if (input_data.isKeyPressed(GLFW_KEY_LEFT_SHIFT)) {
    velocity.y = -0.25f;
  }
  else if (input_data.isKeyPressed(GLFW_KEY_LEFT_CONTROL)) {
    velocity.y = 0.25f;
  }
  else {
    velocity.y = 0;
  }
  // Z
  if (input_data.isKeyPressed(GLFW_KEY_W)) {
    velocity.z = -0.5f;
  }
  else if (input_data.isKeyPressed(GLFW_KEY_S)) {
    velocity.z = 0.5f;
  }
  else {
    velocity.z = 0;
  }

  position_ +=
    Vector3f{ getRotationMatrix() * Vector4f{ velocity * 0.5f, 0.f } };

  if (input_data.isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT)) {
    auto diff = input_data.calculateCursorPositionDelta();
    yaw_ -= diff[0] / 300.f;
    pitch_ += diff[1] / 300.f;
  }
}

NAMESPACE_END(eldr)
