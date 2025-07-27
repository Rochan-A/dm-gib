#pragma once

#include "util/macros.h"
#include "util/report/report.h"
#include <array>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "engine/core/types.h"

// Number of keys.
static constexpr int kNumKeys = GLFW_KEY_LAST + 1;
// Number of mouse buttons.
static constexpr int kNumMouseButtons = GLFW_MOUSE_BUTTON_LAST + 1;

namespace gib {

// Behavior of Left Mouse Button
enum class MouseButtonBehavior : unsigned char {
  NONE = 0,
  CAPTURE = 1,
};

// Behavior when Esc key is pressed.
enum class EscBehavior : unsigned char {
  NONE = 0,
  // Uncaptures the mouse if it is captured.
  TOGGLE_MOUSE_CAPTURE = 1,
  // Close the app.
  CLOSE = 2,
  // Uncaptures the mouse if it is captured, or else closes the app.
  UNCAPTURE_MOUSE_OR_CLOSE = 3,
};

// Mouse button states.
enum class MouseButtonAction : unsigned char {
  UNKNOWN,
  RELEASE,
  PRESS,
};

// Key states.
enum class KeyAction : unsigned char { UNKNOWN, RELEASE, PRESS, REPEAT };

struct Input {
  std::array<KeyAction, kNumKeys> key_state{};
  std::array<MouseButtonAction, kNumMouseButtons> mouse_button_state{};

  Offset mouse_pos{};
  Offset scroll_offset{};

  void Reset() { scroll_offset = {0.f, 0.f}; }

  void KeyCallback(int key, int /*scancode*/, int action, int /*mods*/) {
    key_state[key] = static_cast<KeyAction>(action);
  }

  void MouseButtonCallback(int button, int action, int /*mods*/) {
    mouse_button_state[button] = static_cast<MouseButtonAction>(action);
  }

  void ScrollCallback(const double xoffset, const double yoffset) {
    scroll_offset = {static_cast<float>(xoffset), static_cast<float>(yoffset)};
  }

  void MouseMoveCallback(const double xpos, const double ypos) {
    mouse_pos = {static_cast<float>(xpos), static_cast<float>(ypos)};
  }
};

} // namespace gib