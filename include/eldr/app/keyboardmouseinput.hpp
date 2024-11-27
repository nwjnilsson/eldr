#pragma once
#include <GLFW/glfw3.h>

#include <array>
#include <shared_mutex>

namespace eldr::app::input {
class KeyboardMouseInput {
public:
  KeyboardMouseInput()                          = default;
  KeyboardMouseInput(const KeyboardMouseInput&) = delete;
  KeyboardMouseInput(KeyboardMouseInput&&)      = delete;
  ~KeyboardMouseInput()                         = default;

  KeyboardMouseInput& operator=(const KeyboardMouseInput&) = delete;
  KeyboardMouseInput& operator=(KeyboardMouseInput&&)      = delete;

  /// @brief Change the key's state to pressed.
  /// @param key the key which was pressed and greater or equal to 0
  void pressKey(int32_t key);

  /// @brief Change the key's state to unpressed.
  /// @param key the key which was released
  /// @note key must be smaller than ``GLFW_KEY_LAST`` and greater or equal to 0
  void releaseKey(int32_t key);

  /// @brief Check if the given key is currently pressed.
  /// @param key the key index
  /// @note key must be smaller than ``GLFW_KEY_LAST`` and greater or equal to 0
  /// @return ``true`` if the key is pressed
  [[nodiscard]] bool isKeyPressed(int32_t key) const;

  /// @brief Checks if a key was pressed once.
  /// @param key The key index
  /// @note key must be smaller than ``GLFW_KEY_LAST`` and greater or equal to 0
  /// @return ``true`` if the key was pressed
  [[nodiscard]] bool wasKeyPressedOnce(int32_t key);

  /// @brief Change the mouse button's state to pressed.
  /// @param button the mouse button which was pressed
  /// @note button must be smaller than ``GLFW_MOUSE_BUTTON_LAST`` and greater
  /// or equal to 0
  void pressMouseButton(int32_t button);

  /// @brief Change the mouse button's state to unpressed.
  /// @param button the mouse button which was released
  /// @note button must be smaller than ``GLFW_MOUSE_BUTTON_LAST`` and greater
  /// or equal to 0
  void releaseMouseButton(int32_t button);

  /// @brief Check if the given mouse button is currently pressed.
  /// @param button the mouse button index
  /// @note button must be smaller than ``GLFW_MOUSE_BUTTON_LAST`` and greater
  /// or equal to 0
  /// @return ``true`` if the mouse button is pressed
  [[nodiscard]] bool isMouseButtonPressed(int32_t button) const;

  /// @brief Checks if a mouse button was pressed once.
  /// @param button the mouse button index
  /// @note button must be smaller than ``GLFW_MOUSE_BUTTON_LAST`` and greater
  /// or equal to 0
  /// @return ``true`` if the mouse button was pressed
  [[nodiscard]] bool wasMouseButtonPressedOnce(int32_t button);

  /// @brief Set the current cursor position.
  /// @param pos_x the current x-coordinate of the cursor
  /// @param pos_y the current y-coordinate of the cursor
  void setCursorPos(double pos_x, double pos_y);

  [[nodiscard]] std::array<int64_t, 2> cursorPos() const;

  /// @brief Calculate the change in x- and y-position of the cursor.
  /// @return a std::array of size 2 which contains the change in x-position in
  /// index 0 and the change in y-position in index 1
  [[nodiscard]] std::array<double, 2> calculateCursorPositionDelta();

private:
  std::array<int64_t, 2>                   previous_cursor_pos_{ 0, 0 };
  std::array<int64_t, 2>                   current_cursor_pos_{ 0, 0 };
  std::array<bool, GLFW_KEY_LAST>          pressed_keys_{ false };
  std::array<bool, GLFW_MOUSE_BUTTON_LAST> pressed_mouse_buttons_{ false };
  bool                                     keyboard_updated_{ false };
  bool                                     mouse_buttons_updated_{ false };
  mutable std::shared_mutex                input_mutex_;
};
} // namespace eldr::app::input
