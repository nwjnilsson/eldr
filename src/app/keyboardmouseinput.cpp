#include <app/keyboardmouseinput.hpp>
#include <core/logger.hpp>
#include <core/vector.hpp>

#include <GLFW/glfw3.h>

#include <array>
#include <shared_mutex>

NAMESPACE_BEGIN(eldr)
struct KeyboardMouseInput::KeyboardMouseInputData {
  Point2d                                  previous_cursor_pos{ 0, 0 };
  Point2d                                  current_cursor_pos{ 0, 0 };
  std::array<bool, GLFW_KEY_LAST>          pressed_keys{ false };
  std::array<bool, GLFW_MOUSE_BUTTON_LAST> pressed_mouse_buttons{ false };
  bool                                     keyboard_updated{ false };
  bool                                     mouse_buttons_updated{ false };
  mutable std::shared_mutex                input_mutex;
};

KeyboardMouseInput::KeyboardMouseInput()
  : d_(std::make_unique<KeyboardMouseInputData>())
{
}
KeyboardMouseInput::~KeyboardMouseInput() = default;

void KeyboardMouseInput::pressKey(const int key)
{
  Assert(key >= 0);
  Assert(key < GLFW_KEY_LAST);

  std::lock_guard lock(d_->input_mutex);
  d_->pressed_keys[key] = true;
  d_->keyboard_updated  = true;
}

void KeyboardMouseInput::releaseKey(const int key)
{
  Assert(key >= 0);
  Assert(key < GLFW_KEY_LAST);

  std::lock_guard lock(d_->input_mutex);
  d_->pressed_keys[key] = false;
  d_->keyboard_updated  = true;
}

bool KeyboardMouseInput::isKeyPressed(const int key) const
{
  Assert(key >= 0);
  Assert(key < GLFW_KEY_LAST);

  std::shared_lock lock(d_->input_mutex);
  return d_->pressed_keys[key];
}

bool KeyboardMouseInput::wasKeyPressedOnce(const int key)
{
  Assert(key >= 0);
  Assert(key < GLFW_KEY_LAST);

  std::lock_guard lock(d_->input_mutex);
  if (not d_->pressed_keys[key] or not d_->keyboard_updated) {
    return false;
  }

  d_->pressed_keys[key] = false;
  return true;
}

void KeyboardMouseInput::pressMouseButton(const int button)
{
  Assert(button >= 0);
  Assert(button < GLFW_MOUSE_BUTTON_LAST);

  std::lock_guard lock(d_->input_mutex);
  d_->pressed_mouse_buttons[button] = true;
  d_->mouse_buttons_updated         = true;
}

void KeyboardMouseInput::releaseMouseButton(const int button)
{
  Assert(button >= 0);
  Assert(button < GLFW_MOUSE_BUTTON_LAST);

  std::lock_guard lock(d_->input_mutex);
  d_->pressed_mouse_buttons[button] = false;
  d_->mouse_buttons_updated         = true;
}

bool KeyboardMouseInput::isMouseButtonPressed(const int button) const
{
  Assert(button >= 0);
  Assert(button < GLFW_MOUSE_BUTTON_LAST);

  std::shared_lock lock(d_->input_mutex);
  return d_->pressed_mouse_buttons[button];
}

bool KeyboardMouseInput::wasMouseButtonPressedOnce(const int button)
{
  Assert(button >= 0);
  Assert(button < GLFW_MOUSE_BUTTON_LAST);

  std::lock_guard lock(d_->input_mutex);
  if (not d_->pressed_mouse_buttons[button] or not d_->mouse_buttons_updated) {
    return false;
  }

  d_->pressed_mouse_buttons[button] = false;
  return true;
}

void KeyboardMouseInput::setCursorPos(const double pos_x, const double pos_y)
{
  std::lock_guard lock(d_->input_mutex);
  d_->previous_cursor_pos   = d_->current_cursor_pos;
  d_->current_cursor_pos[0] = pos_x;
  d_->current_cursor_pos[1] = pos_y;
}

KeyboardMouseInput::Point2d KeyboardMouseInput::cursorPos() const
{
  std::lock_guard lock(d_->input_mutex);
  return d_->current_cursor_pos;
}

KeyboardMouseInput::Point2d
KeyboardMouseInput::calculateCursorPositionDelta() const
{
  std::lock_guard lock(d_->input_mutex);
  // Calculate the change in cursor position in x- and y-axis.
  const Point2d cursor_pos_delta{
    d_->current_cursor_pos[0] - d_->previous_cursor_pos[0],
    d_->current_cursor_pos[1] - d_->previous_cursor_pos[1]
  };
  d_->previous_cursor_pos = d_->current_cursor_pos;
  return cursor_pos_delta;
}

NAMESPACE_END(eldr)
