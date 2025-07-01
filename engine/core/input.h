#pragma once

#include "util/macros.h"
#include "util/report/macros.h"
#include <array>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "engine/core/types.h"

// Number of keys.
static constexpr int kNumKeys = GLFW_KEY_LAST + 1;
// Number of mouse buttons.
static constexpr int kNumMouseButtons = GLFW_MOUSE_BUTTON_LAST + 1;

namespace gib {

enum class MouseButtonBehavior : unsigned char {
  NONE,
  CAPTURE,
};

// Controls behavior when Esc key is pressed.
enum class EscBehavior : unsigned char {
  NONE,
  TOGGLE_MOUSE_CAPTURE,
  CLOSE,
  // Uncaptures the mouse if it is captured, or else closes the app.
  UNCAPTURE_MOUSE_OR_CLOSE,
};

enum class MouseButtonAction : unsigned char {
  UNKNOWN,
  PRESS,
  RELEASE,
};

enum class KeyAction : unsigned char { UNKNOWN, PRESS, REPEAT, RELEASE };

struct Input {
  std::array<KeyAction, kNumKeys> key_state{};
  std::array<MouseButtonAction, kNumMouseButtons> mouse_button_state{};

  Offset mouse_pos{};
  Offset scroll_offset{};

  void KeyCallback(int key, int scancode, int action, int mods) {
    key_state[key] = KeyAction(action);
  }

  void MouseButtonCallback(int button, int action, int mods) {
    mouse_button_state[button] = MouseButtonAction(action);
  }

  void ScrollCallback(double xoffset, double yoffset) {
    scroll_offset.x = static_cast<float>(xoffset);
    scroll_offset.y = static_cast<float>(yoffset);
  }

  void MouseMoveCallback(double xpos, double ypos) {
    mouse_pos.x = static_cast<float>(xpos);
    mouse_pos.y = static_cast<float>(ypos);
  }
};

class InputManager {
public:
  InputManager(GLFWwindow *glfw_window_ptr,
               MouseButtonBehavior left_mouse_button_behavior,
               EscBehavior esc_behavior)
      : mouse_button_behavior_(left_mouse_button_behavior),
        esc_behavior_(esc_behavior), glfw_window_ptr_(glfw_window_ptr) {}

  ~InputManager() {}

  DISALLOW_COPY_AND_ASSIGN(InputManager);

  void EnableMouseCapture() {
    glfwSetInputMode(glfw_window_ptr_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    INFO("Enabled mouse capture.");
  }

  void DisableMouseCapture() {
    glfwSetInputMode(glfw_window_ptr_, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    INFO("Disabled mouse capture.");
  }

  void EnableKeyInput() {
    if (key_input_enabled_) {
      return;
    }
    auto callback = [](GLFWwindow *window, int key, int scancode, int action,
                       int mods) {
      auto self =
          reinterpret_cast<InputManager *>(glfwGetWindowUserPointer(window));
      self->KeyCallback(key, scancode, action, mods);
    };
    glfwSetKeyCallback(glfw_window_ptr_, callback);
    key_input_enabled_ = true;
    INFO("Enabled Window key input.");
  }

  void DisableKeyInput() {
    if (!key_input_enabled_) {
      return;
    }
    glfwSetKeyCallback(glfw_window_ptr_, nullptr);
    key_input_enabled_ = false;
    INFO("Disabled Window key input.");
  }

  void EnableScrollInput() {
    if (scroll_input_enabled_) {
      return;
    }
    auto callback = [](GLFWwindow *window, double xoffset, double yoffset) {
      auto self =
          reinterpret_cast<InputManager *>(glfwGetWindowUserPointer(window));
      self->ScrollCallback(xoffset, yoffset);
    };
    glfwSetScrollCallback(glfw_window_ptr_, callback);
    scroll_input_enabled_ = true;
    INFO("Enabled Window scroll input.");
  }

  void DisableScrollInput() {
    if (!scroll_input_enabled_) {
      return;
    }
    glfwSetScrollCallback(glfw_window_ptr_, nullptr);
    scroll_input_enabled_ = false;
    INFO("Disable Window scroll input.");
  }

  void EnableMouseMoveInput() {
    if (mouse_move_input_enabled_) {
      return;
    }
    auto callback = [](GLFWwindow *window, double xpos, double ypos) {
      auto self =
          reinterpret_cast<InputManager *>(glfwGetWindowUserPointer(window));
      self->MouseMoveCallback(xpos, ypos);
    };
    glfwSetCursorPosCallback(glfw_window_ptr_, callback);
    mouse_move_input_enabled_ = true;
    INFO("Enable Window mouse move input.");
  }

  void DisableMouseMoveInput() {
    if (!mouse_move_input_enabled_) {
      return;
    }
    glfwSetCursorPosCallback(glfw_window_ptr_, nullptr);
    mouse_move_input_enabled_ = false;
    INFO("Disable Window mouse move input.");
  }

  void EnableMouseButtonInput() {
    if (mouse_button_input_enabled_) {
      return;
    }
    auto callback = [](GLFWwindow *window, int button, int action, int mods) {
      auto self =
          reinterpret_cast<InputManager *>(glfwGetWindowUserPointer(window));
      self->MouseButtonCallback(button, action, mods);
    };
    glfwSetMouseButtonCallback(glfw_window_ptr_, callback);
    mouse_button_input_enabled_ = true;
    INFO("Enable Window mouse button input.");
  }

  void DisableMouseButtonInput() {
    if (!mouse_button_input_enabled_) {
      return;
    }
    glfwSetMouseButtonCallback(glfw_window_ptr_, nullptr);
    mouse_button_input_enabled_ = false;
    INFO("Disable Window mouse button input.");
  }

  void KeyCallback(int key, int scancode, int action, int mods) {
    if (key_input_paused_) {
      return;
    }

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
      if (esc_behavior_ == EscBehavior::TOGGLE_MOUSE_CAPTURE) {
        auto inputMode = glfwGetInputMode(glfw_window_ptr_, GLFW_CURSOR);
        if (inputMode == GLFW_CURSOR_NORMAL) {
          EnableMouseCapture();
        } else {
          DisableMouseCapture();
        }
      } else if (esc_behavior_ == EscBehavior::CLOSE) {
        glfwSetWindowShouldClose(glfw_window_ptr_, true);
      } else if (esc_behavior_ == EscBehavior::UNCAPTURE_MOUSE_OR_CLOSE) {
        auto inputMode = glfwGetInputMode(glfw_window_ptr_, GLFW_CURSOR);
        if (inputMode == GLFW_CURSOR_DISABLED) {
          DisableMouseCapture();
        } else {
          // Close since mouse is not captured.
          glfwSetWindowShouldClose(glfw_window_ptr_, true);
        }
      }
    }

    inputs.KeyCallback(key, scancode, action, mods);
  }

  void ScrollCallback(double xoffset, double yoffset) {
    if (mouse_input_paused_) {
      return;
    }
    inputs.ScrollCallback(xoffset, yoffset);
  }

  void MouseMoveCallback(double xpos, double ypos) {
    if (mouse_input_paused_) {
      return;
    }
    inputs.MouseMoveCallback(xpos, ypos);
  }

  void MouseButtonCallback(int button, int action, int mods) {
    if (mouse_input_paused_) {
      return;
    }

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
      if (mouse_button_behavior_ == MouseButtonBehavior::CAPTURE) {
        EnableMouseCapture();
      }
    }
    inputs.MouseButtonCallback(button, action, mods);
  }

  void SetGLFWInputMode(const int mode, const int value) {
    glfwSetInputMode(glfw_window_ptr_, mode, value);
    INFO("Set GLFW InputMode to mode: {}, value: {}", mode, value);
  }

  Input inputs{};

  bool mouse_captured_{false};
  bool key_input_enabled_{false};
  bool scroll_input_enabled_{false};
  bool mouse_move_input_enabled_{false};
  bool mouse_button_input_enabled_{false};
  bool mouse_input_paused_{false};
  bool key_input_paused_{false};

  MouseButtonBehavior mouse_button_behavior_{};
  EscBehavior esc_behavior_{};

private:
  GLFWwindow *glfw_window_ptr_{nullptr};
};

} // namespace gib