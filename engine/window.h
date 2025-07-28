#pragma once

#include "engine/core/gl_core.h"
#include "engine/core/gl_window.h"
#include "engine/core/input.h"
#include "util/imgui/imgui_util.h"
#include "util/imgui/imgui_window.h"
#include "util/macros.h"
#include "util/report/report.h"

static constexpr glm::vec4 kDefaultClearColor =
    glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

namespace gib {

// Owns OS window and drives render loop.
template <typename WindowImpl> class WindowBase {
public:
  explicit WindowBase(const std::string name)
      : gl_core_(std::make_shared<GLCore>()), gl_window_(gl_core_, name),
        imgui_window_(gl_window_.GetGlfwWindowPtr()) {
    // Allow us to refer to this WindowImpl object while accessing C APIs.
    glfwSetWindowUserPointer(gl_window_.GetGlfwWindowPtr(), this);
  }
  ~WindowBase() = default;

  // Input related toggles.
  void ToggleMouseCapture(const bool &enable_mouse_capture);
  void ToggleKeyInput(const bool &enable_key_input);
  void ToggleScrollInput(const bool &enable_scroll_input);
  void ToggleMouseMoveInput(const bool &enable_mouse_move_input);
  void ToggleMouseButtonInput(const bool &enable_mouse_button_input);

  // Set ESC key and LMB behavior.
  void SetEscKeyBehavior(EscBehavior esc_behavior);
  void SetMouseButtonBehavior(MouseButtonBehavior mouse_button_behavior);
  // Set GLFW input mode.
  void SetGLFWInputMode(const int mode = GLFW_CURSOR,
                        const int value = GLFW_CURSOR_DISABLED);

  [[nodiscard]] const Input &GetInput() const { return input_; }

  // Enter the main loop. This call blocks until the user closes the window or
  // the application requests shutdown (glfwSetWindowShouldClose()).
  void Run();

  DISALLOW_COPY_AND_ASSIGN(WindowBase);

  // Context for the app window.
  struct WindowContext {
    bool enable_mouse_capture{false};
    bool enable_key_input{false};
    bool enable_scroll_input{false};
    bool enable_mouse_move_input{false};
    bool enable_mouse_button_input{false};
  };

  WindowContext ctx_;

private:
  void DebugUI();

  void KeyCallback(int key, int scancode, int action, int mods);
  void MouseButtonCallback(int button, int action, int mods);
  void ScrollCallback(double xoffset, double yoffset);
  void MouseMoveCallback(double xpos, double ypos);

  Input input_{};
  MouseButtonBehavior mouse_button_behavior_{};
  EscBehavior esc_behavior_{};

  std::shared_ptr<GLCore> gl_core_;
  GlfwWindow gl_window_;
  time_util::TimePoint last_time_;

  imgui_util::ImGuiWindow imgui_window_;
  glm::vec4 clear_color_ = kDefaultClearColor;
};

} // namespace gib
