#pragma once
#include <eldr/eldr.hpp>

#include <memory>

NAMESPACE_BEGIN(eldr::app)
class KeyboardMouseInput {
public:
  KeyboardMouseInput();
  KeyboardMouseInput(const KeyboardMouseInput&) = delete;
  KeyboardMouseInput(KeyboardMouseInput&&)      = delete;
  ~KeyboardMouseInput();

  KeyboardMouseInput& operator=(const KeyboardMouseInput&) = delete;
  KeyboardMouseInput& operator=(KeyboardMouseInput&&)      = delete;

  /// @brief Change the key's state to pressed.
  /// @param key the key which was pressed and greater or equal to 0
  void pressKey(int key);

  /// @brief Change the key's state to unpressed.
  /// @param key the key which was released
  /// @note key must be smaller than ``GLFW_KEY_LAST`` and greater or equal to 0
  void releaseKey(int key);

  /// @brief Check if the given key is currently pressed.
  /// @param key the key index
  /// @note key must be smaller than ``GLFW_KEY_LAST`` and greater or equal to 0
  /// @return ``true`` if the key is pressed
  [[nodiscard]] bool isKeyPressed(int key) const;

  /// @brief Checks if a key was pressed once.
  /// @param key The key index
  /// @note key must be smaller than ``GLFW_KEY_LAST`` and greater or equal to 0
  /// @return ``true`` if the key was pressed
  [[nodiscard]] bool wasKeyPressedOnce(int key);

  /// @brief Change the mouse button's state to pressed.
  /// @param button the mouse button which was pressed
  /// @note button must be smaller than ``GLFW_MOUSE_BUTTON_LAST`` and greater
  /// or equal to 0
  void pressMouseButton(int button);

  /// @brief Change the mouse button's state to unpressed.
  /// @param button the mouse button which was released
  /// @note button must be smaller than ``GLFW_MOUSE_BUTTON_LAST`` and greater
  /// or equal to 0
  void releaseMouseButton(int button);

  /// @brief Check if the given mouse button is currently pressed.
  /// @param button the mouse button index
  /// @note button must be smaller than ``GLFW_MOUSE_BUTTON_LAST`` and greater
  /// or equal to 0
  /// @return ``true`` if the mouse button is pressed
  [[nodiscard]] bool isMouseButtonPressed(int button) const;

  /// @brief Checks if a mouse button was pressed once.
  /// @param button the mouse button index
  /// @note button must be smaller than ``GLFW_MOUSE_BUTTON_LAST`` and greater
  /// or equal to 0
  /// @return ``true`` if the mouse button was pressed
  [[nodiscard]] bool wasMouseButtonPressedOnce(int button);

  /// @brief Set the current cursor position.
  /// @param pos_x the current x-coordinate of the cursor
  /// @param pos_y the current y-coordinate of the cursor
  void setCursorPos(double pos_x, double pos_y);

  [[nodiscard]] std::array<double, 2> cursorPos() const;

  /// @brief Calculate the change in x- and y-position of the cursor.
  /// @return a std::array of size 2 which contains the change in x-position in
  /// index 0 and the change in y-position in index 1
  [[nodiscard]] std::array<double, 2> calculateCursorPositionDelta();

private:
  struct KeyboardMouseInputData;
  std::unique_ptr<KeyboardMouseInputData> d_;
};
NAMESPACE_END(eldr::app)
