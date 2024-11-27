#include <eldr/app/keyboardmouseinput.hpp>

#include <cassert>
#include <mutex>

namespace eldr::app::input {

void KeyboardMouseInput::pressKey(const int32_t key)
{
  assert(key >= 0);
  assert(key < GLFW_KEY_LAST);

  std::lock_guard lock(input_mutex_);
  pressed_keys_[key] = true;
  keyboard_updated_  = true;
}

void KeyboardMouseInput::releaseKey(const int32_t key)
{
  assert(key >= 0);
  assert(key < GLFW_KEY_LAST);

  std::lock_guard lock(input_mutex_);
  pressed_keys_[key] = false;
  keyboard_updated_  = true;
}

bool KeyboardMouseInput::isKeyPressed(const int32_t key) const
{
  assert(key >= 0);
  assert(key < GLFW_KEY_LAST);

  std::shared_lock lock(input_mutex_);
  return pressed_keys_[key];
}

bool KeyboardMouseInput::wasKeyPressedOnce(const int32_t key)
{
  assert(key >= 0);
  assert(key < GLFW_KEY_LAST);

  std::lock_guard lock(input_mutex_);
  if (!pressed_keys_[key] || !keyboard_updated_) {
    return false;
  }

  pressed_keys_[key] = false;
  return true;
}

void KeyboardMouseInput::pressMouseButton(const int32_t button)
{
  assert(button >= 0);
  assert(button < GLFW_MOUSE_BUTTON_LAST);

  std::lock_guard lock(input_mutex_);
  pressed_mouse_buttons_[button] = true;
  mouse_buttons_updated_         = true;
}

void KeyboardMouseInput::releaseMouseButton(const int32_t button)
{
  assert(button >= 0);
  assert(button < GLFW_MOUSE_BUTTON_LAST);

  std::lock_guard lock(input_mutex_);
  pressed_mouse_buttons_[button] = false;
  mouse_buttons_updated_         = true;
}

bool KeyboardMouseInput::isMouseButtonPressed(const int32_t button) const
{
  assert(button >= 0);
  assert(button < GLFW_MOUSE_BUTTON_LAST);

  std::shared_lock lock(input_mutex_);
  return pressed_mouse_buttons_[button];
}

bool KeyboardMouseInput::wasMouseButtonPressedOnce(const int32_t button)
{
  assert(button >= 0);
  assert(button < GLFW_MOUSE_BUTTON_LAST);

  std::lock_guard lock(input_mutex_);
  if (!pressed_mouse_buttons_[button] || !mouse_buttons_updated_) {
    return false;
  }

  pressed_mouse_buttons_[button] = false;
  return true;
}

void KeyboardMouseInput::setCursorPos(const double pos_x, const double pos_y)
{
  std::lock_guard lock(input_mutex_);
  current_cursor_pos_[0] = static_cast<int64_t>(pos_x);
  current_cursor_pos_[1] = static_cast<int64_t>(pos_y);
}

std::array<int64_t, 2> KeyboardMouseInput::cursorPos() const
{
  std::shared_lock lock(input_mutex_);
  return current_cursor_pos_;
}

std::array<double, 2> KeyboardMouseInput::calculateCursorPositionDelta()
{
  std::lock_guard lock(input_mutex_);
  // Calculate the change in cursor position in x- and y-axis.
  const std::array cursor_pos_delta{
    static_cast<double>(current_cursor_pos_[0]) -
      static_cast<double>(previous_cursor_pos_[0]),
    static_cast<double>(current_cursor_pos_[1]) -
      static_cast<double>(previous_cursor_pos_[1])
  };

  previous_cursor_pos_ = current_cursor_pos_;

  return cursor_pos_delta;
}

} // namespace eldr::app::input
