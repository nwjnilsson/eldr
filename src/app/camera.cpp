#include "camera.hpp"
#include "keyboardmouseinput.hpp"

#include <core/transform.hpp>

#include <GLFW/glfw3.h>

NAMESPACE_BEGIN(eldr)
Camera::Camera() = default;

void Camera::processInput(const KeyboardMouseInput& input_data)
{
  // X
  Vector3f velocity{ 0.f };
  if (input_data.isKeyPressed(GLFW_KEY_A)) {
    velocity.x() = -0.5f;
  }
  else if (input_data.isKeyPressed(GLFW_KEY_D)) {
    velocity.x() = 0.5f;
  }
  else {
    velocity.x() = 0;
  }
  // Y
  if (input_data.isKeyPressed(GLFW_KEY_LEFT_SHIFT)) {
    velocity.y() = -0.25f;
  }
  else if (input_data.isKeyPressed(GLFW_KEY_LEFT_CONTROL)) {
    velocity.y() = 0.25f;
  }
  else {
    velocity.y() = 0;
  }
  // Z
  if (input_data.isKeyPressed(GLFW_KEY_W)) {
    velocity.z() = -0.5f;
  }
  else if (input_data.isKeyPressed(GLFW_KEY_S)) {
    velocity.z() = 0.5f;
  }
  else {
    velocity.z() = 0;
  }

  position_ += rotation() * (velocity * 0.5f);

  if (input_data.isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT)) {
    auto diff = input_data.calculateCursorPositionDelta();
    yaw_ -= diff[0] / 300.f;
    pitch_ += diff[1] / 300.f;
  }
}
NAMESPACE_END(eldr)
