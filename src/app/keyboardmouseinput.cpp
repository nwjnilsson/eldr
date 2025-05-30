#include <eldr/app/keyboardmouseinput.hpp>
#include <eldr/core/logger.hpp>

#include <GLFW/glfw3.h>

#include <array>
#include <shared_mutex>

namespace eldr::app {
struct KeyboardMouseInput::KeyboardMouseInputData {
  std::array<int64_t, 2>                   previous_cursor_pos{ 0, 0 };
  std::array<int64_t, 2>                   current_cursor_pos{ 0, 0 };
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

void KeyboardMouseInput::pressKey(const int32_t key)
{
  Assert(key >= 0);
  Assert(key < GLFW_KEY_LAST);

  std::lock_guard lock(d_->input_mutex);
  d_->pressed_keys[key] = true;
  d_->keyboard_updated  = true;
}

void KeyboardMouseInput::releaseKey(const int32_t key)
{
  Assert(key >= 0);
  Assert(key < GLFW_KEY_LAST);

  std::lock_guard lock(d_->input_mutex);
  d_->pressed_keys[key] = false;
  d_->keyboard_updated  = true;
}

bool KeyboardMouseInput::isKeyPressed(const int32_t key) const
{
  Assert(key >= 0);
  Assert(key < GLFW_KEY_LAST);

  std::shared_lock lock(d_->input_mutex);
  return d_->pressed_keys[key];
}

bool KeyboardMouseInput::wasKeyPressedOnce(const int32_t key)
{
  Assert(key >= 0);
  Assert(key < GLFW_KEY_LAST);

  std::lock_guard lock(d_->input_mutex);
  if (!d_->pressed_keys[key] || !d_->keyboard_updated) {
    return false;
  }

  d_->pressed_keys[key] = false;
  return true;
}

void KeyboardMouseInput::pressMouseButton(const int32_t button)
{
  Assert(button >= 0);
  Assert(button < GLFW_MOUSE_BUTTON_LAST);

  std::lock_guard lock(d_->input_mutex);
  d_->pressed_mouse_buttons[button] = true;
  d_->mouse_buttons_updated         = true;
}

void KeyboardMouseInput::releaseMouseButton(const int32_t button)
{
  Assert(button >= 0);
  Assert(button < GLFW_MOUSE_BUTTON_LAST);

  std::lock_guard lock(d_->input_mutex);
  d_->pressed_mouse_buttons[button] = false;
  d_->mouse_buttons_updated         = true;
}

bool KeyboardMouseInput::isMouseButtonPressed(const int32_t button) const
{
  Assert(button >= 0);
  Assert(button < GLFW_MOUSE_BUTTON_LAST);

  std::shared_lock lock(d_->input_mutex);
  return d_->pressed_mouse_buttons[button];
}

bool KeyboardMouseInput::wasMouseButtonPressedOnce(const int32_t button)
{
  Assert(button >= 0);
  Assert(button < GLFW_MOUSE_BUTTON_LAST);

  std::lock_guard lock(d_->input_mutex);
  if (!d_->pressed_mouse_buttons[button] || !d_->mouse_buttons_updated) {
    return false;
  }

  d_->pressed_mouse_buttons[button] = false;
  return true;
}

void KeyboardMouseInput::setCursorPos(const double pos_x, const double pos_y)
{
  std::lock_guard lock(d_->input_mutex);
  d_->current_cursor_pos[0] = static_cast<int64_t>(pos_x);
  d_->current_cursor_pos[1] = static_cast<int64_t>(pos_y);
}

std::array<int64_t, 2> KeyboardMouseInput::cursorPos() const
{
  std::shared_lock lock(d_->input_mutex);
  return d_->current_cursor_pos;
}

std::array<double, 2> KeyboardMouseInput::calculateCursorPositionDelta()
{
  std::lock_guard lock(d_->input_mutex);
  // Calculate the change in cursor position in x- and y-axis.
  const std::array cursor_pos_delta{
    static_cast<double>(d_->current_cursor_pos[0]) -
      static_cast<double>(d_->previous_cursor_pos[0]),
    static_cast<double>(d_->current_cursor_pos[1]) -
      static_cast<double>(d_->previous_cursor_pos[1])
  };

  d_->previous_cursor_pos = d_->current_cursor_pos;

  return cursor_pos_delta;
}

} // namespace eldr::app
