#include <eldr/app/keyboardmouseinput.hpp>
#include <eldr/render/camera.hpp>

#include <GLFW/glfw3.h>

NAMESPACE_BEGIN(eldr)
using Matrix4f = ProjectiveCamera::Matrix4f;

ProjectiveCamera::ProjectiveCamera() = default;

Matrix4f ProjectiveCamera::getRotationMatrix() const
{
  Quat4f pitch_rot{ glm::angleAxis(pitch_, Vector3f{ 1.f, 0.f, 0.f }) };
  Quat4f yaw_rot{ glm::angleAxis(yaw_, Vector3f{ 0.f, -1.f, 0.f }) };
  return glm::toMat4(yaw_rot) * glm::toMat4(pitch_rot);
}

Matrix4f PerspectiveCamera::getViewMatrix() const
{
  Matrix4f translation{ glm::translate(Matrix4f{ 1.f }, position_) };
  return glm::inverse(translation * getRotationMatrix());
}

Camera::Camera() = default;

void Camera::update()
{
  position_ +=
    Vector3f{ getRotationMatrix() * Vector4f{ velocity_ * 0.5f, 0.f } };
}

void Camera::processInput(const KeyboardMouseInput& input_data)
{
  // X
  if (input_data.isKeyPressed(GLFW_KEY_A)) {
    velocity_.x = -1;
  }
  else if (input_data.isKeyPressed(GLFW_KEY_D)) {
    velocity_.x = 1;
  }
  else {
    velocity_.x = 0;
  }
  // Z
  if (input_data.isKeyPressed(GLFW_KEY_W)) {
    velocity_.z = -1;
  }
  else if (input_data.isKeyPressed(GLFW_KEY_S)) {
    velocity_.z = 1;
  }
  else {
    velocity_.z = 0;
  }

  if (input_data.isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT)) {
    auto diff = input_data.calculateCursorPositionDelta();
    yaw_ += diff[0] / 200.f;
    pitch_ += diff[1] / 200.f;
  }
}

NAMESPACE_END(eldr)
