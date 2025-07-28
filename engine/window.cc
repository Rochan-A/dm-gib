#include "engine/window.h"
#include "engine/core/gl_core.h"
#include "engine/core/gl_window.h"
#include "engine/core/input.h"
#include "util/imgui/imgui_util.h"
#include "util/imgui/imgui_window.h"
#include "util/macros.h"
#include "util/report/report.h"

namespace gib {

template <typename WindowImpl>
void WindowBase<WindowImpl>::ToggleMouseCapture(
    const bool &enable_mouse_capture) {
  if (ctx_.enable_mouse_capture == enable_mouse_capture) {
    return;
  }
  if (enable_mouse_capture) {
    glfwSetInputMode(gl_window_.GetGlfwWindowPtr(), GLFW_CURSOR,
                     GLFW_CURSOR_DISABLED);
  } else {
    glfwSetInputMode(gl_window_.GetGlfwWindowPtr(), GLFW_CURSOR,
                     GLFW_CURSOR_NORMAL);
  }
  ctx_.enable_mouse_capture = enable_mouse_capture;
  DEBUG("Mouse capture enabled: {}", ctx_.enable_mouse_capture);
}

template <typename WindowImpl>
void WindowBase<WindowImpl>::ToggleKeyInput(const bool &enable_key_input) {
  if (ctx_.enable_key_input == enable_key_input) {
    return;
  }
  if (enable_key_input) {
    auto callback = [](GLFWwindow *window, int key, int scancode, int action,
                       int mods) {
      ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
      // We use WantCaptureMouse here because it is true if the mouse is over
      // the Imgui window.
      // TODO(rochan): Fix this.
      ImGuiIO const &imgui_io = ImGui::GetIO();
      if (!imgui_io.WantCaptureMouse ||
          !ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) ||
          !ImGui::IsAnyItemActive()) {
        auto self =
            reinterpret_cast<WindowImpl *>(glfwGetWindowUserPointer(window));
        self->KeyCallback(key, scancode, action, mods);
      }
    };
    glfwSetKeyCallback(gl_window_.GetGlfwWindowPtr(), callback);
  } else {
    glfwSetKeyCallback(gl_window_.GetGlfwWindowPtr(), nullptr);
  }
  ctx_.enable_key_input = enable_key_input;
  DEBUG("Window key input enabled {}", ctx_.enable_key_input);
}

template <typename WindowImpl>
void WindowBase<WindowImpl>::ToggleScrollInput(
    const bool &enable_scroll_input) {
  if (ctx_.enable_scroll_input == enable_scroll_input) {
    return;
  }
  if (enable_scroll_input) {
    auto callback = [](GLFWwindow *window, double xoffset, double yoffset) {
      ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
      const ImGuiIO &imgui_io = ImGui::GetIO();
      if (!imgui_io.WantCaptureMouse ||
          !ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) ||
          !ImGui::IsAnyItemActive()) {
        auto self =
            reinterpret_cast<WindowImpl *>(glfwGetWindowUserPointer(window));
        self->ScrollCallback(xoffset, yoffset);
      }
    };
    glfwSetScrollCallback(gl_window_.GetGlfwWindowPtr(), callback);
  } else {
    glfwSetScrollCallback(gl_window_.GetGlfwWindowPtr(), nullptr);
  }
  ctx_.enable_scroll_input = enable_scroll_input;
  DEBUG("Window scroll input enabled {}", enable_scroll_input);
}

template <typename WindowImpl>
void WindowBase<WindowImpl>::ToggleMouseMoveInput(
    const bool &enable_mouse_move_input) {
  if (ctx_.enable_mouse_move_input == enable_mouse_move_input) {
    return;
  }
  if (enable_mouse_move_input) {
    auto callback = [](GLFWwindow *window, double xpos, double ypos) {
      ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);
      const ImGuiIO &imgui_io = ImGui::GetIO();
      if (!imgui_io.WantCaptureMouse ||
          !ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) ||
          !ImGui::IsAnyItemActive()) {
        auto self =
            reinterpret_cast<WindowImpl *>(glfwGetWindowUserPointer(window));
        self->MouseMoveCallback(xpos, ypos);
      }
    };
    glfwSetCursorPosCallback(gl_window_.GetGlfwWindowPtr(), callback);
  } else {
    glfwSetCursorPosCallback(gl_window_.GetGlfwWindowPtr(), nullptr);
  }
  ctx_.enable_mouse_move_input = enable_mouse_move_input;
  DEBUG("Window mouse move input enabled: {}", enable_mouse_move_input);
}

template <typename WindowImpl>
void WindowBase<WindowImpl>::ToggleMouseButtonInput(
    const bool &enable_mouse_button_input) {
  if (ctx_.enable_mouse_button_input == enable_mouse_button_input) {
    return;
  }
  if (enable_mouse_button_input) {
    auto callback = [](GLFWwindow *window, int button, int action, int mods) {
      ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
      const ImGuiIO &imgui_io = ImGui::GetIO();
      if (!imgui_io.WantCaptureMouse ||
          !ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) ||
          !ImGui::IsAnyItemActive()) {
        auto self =
            reinterpret_cast<WindowImpl *>(glfwGetWindowUserPointer(window));
        self->MouseButtonCallback(button, action, mods);
      }
    };
    glfwSetMouseButtonCallback(gl_window_.GetGlfwWindowPtr(), callback);
  } else {
    glfwSetMouseButtonCallback(gl_window_.GetGlfwWindowPtr(), nullptr);
  }
  ctx_.enable_mouse_button_input = enable_mouse_button_input;
  DEBUG("Window mouse button input enabled {}", enable_mouse_button_input);
}

template <typename WindowImpl>
void WindowBase<WindowImpl>::KeyCallback(int key, int scancode, int action,
                                         int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    if (esc_behavior_ == EscBehavior::TOGGLE_MOUSE_CAPTURE) {
      ToggleMouseCapture(!ctx_.enable_mouse_capture);
    } else if (esc_behavior_ == EscBehavior::CLOSE) {
      glfwSetWindowShouldClose(gl_window_.GetGlfwWindowPtr(), true);
    } else if (esc_behavior_ == EscBehavior::UNCAPTURE_MOUSE_OR_CLOSE) {
      auto inputMode =
          glfwGetInputMode(gl_window_.GetGlfwWindowPtr(), GLFW_CURSOR);
      if (inputMode == GLFW_CURSOR_DISABLED) {
        ToggleMouseCapture(false);
      } else {
        // Close since mouse is not captured.
        glfwSetWindowShouldClose(gl_window_.GetGlfwWindowPtr(), true);
      }
    }
  }
  input_.KeyCallback(key, scancode, action, mods);
}

template <typename WindowImpl>
void WindowBase<WindowImpl>::ScrollCallback(double xoffset, double yoffset) {
  input_.ScrollCallback(xoffset, yoffset);
}

template <typename WindowImpl>
void WindowBase<WindowImpl>::MouseMoveCallback(double xpos, double ypos) {
  input_.MouseMoveCallback(xpos, ypos);
}

template <typename WindowImpl>
void WindowBase<WindowImpl>::MouseButtonCallback(int button, int action,
                                                 int mods) {
  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
    if (mouse_button_behavior_ == MouseButtonBehavior::CAPTURE) {
      ToggleMouseCapture(true);
    }
  }
  input_.MouseButtonCallback(button, action, mods);
}

template <typename WindowImpl>
void WindowBase<WindowImpl>::SetEscKeyBehavior(const EscBehavior esc_behavior) {
  esc_behavior_ = esc_behavior;
}

template <typename WindowImpl>
void WindowBase<WindowImpl>::SetMouseButtonBehavior(
    const MouseButtonBehavior mouse_button_behavior) {
  mouse_button_behavior_ = mouse_button_behavior;
}

template <typename WindowImpl>
void WindowBase<WindowImpl>::SetGLFWInputMode(const int mode, const int value) {
  glfwSetInputMode(gl_window_.GetGlfwWindowPtr(), mode, value);
  DEBUG("Set GLFW InputMode to mode: {}, value: {}", mode, value);
}

template <typename WindowImpl> void WindowBase<WindowImpl>::Run() {
  bool enable_imgui = true;
  last_time_ = time_util::now();

  while (!glfwWindowShouldClose(gl_window_.GetGlfwWindowPtr())) {
    const FrameTick tick{time_util::now(), time_util::elapsed_usec(last_time_)};
    last_time_ = tick.current_time;
    glfwPollEvents();

    glClearColor(clear_color_.r, clear_color_.g, clear_color_.b,
                 clear_color_.a);
    GLbitfield clear_bits = GL_COLOR_BUFFER_BIT;
    if (gl_window_.GetGlfwWindowContext().enable_depth_test) {
      clear_bits |= GL_DEPTH_BUFFER_BIT;
    }
    if (gl_window_.GetGlfwWindowContext().enable_stencil_test) {
      clear_bits |= GL_STENCIL_BUFFER_BIT;
    }
    glClear(clear_bits);

    gl_window_.Tick(tick);
    static_cast<WindowImpl *>(this)->Tick(tick, gl_window_);
    if (enable_imgui) {
      DebugUI();
    }

    CHECK_GL_ERROR();
    glfwSwapBuffers(gl_window_.GetGlfwWindowPtr());

    static_cast<WindowImpl *>(this)->Tock(tick, gl_window_);
    input_.Reset();
  }
}

template <typename WindowImpl> void WindowBase<WindowImpl>::DebugUI() {
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  // Engine‑level widgets
  if (ImGui::Begin("Debug")) {
    gl_window_.DebugUI();
    ImGui::Separator();
    static_cast<WindowImpl *>(this)->DebugUI(gl_window_);
    ImGui::End();
  }

  // Render frame
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

  // Multi‑viewport support (if enabled in ImGuiIO)
  if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    GLFWwindow *backup = glfwGetCurrentContext();
    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
    glfwMakeContextCurrent(backup);
  }
}

} // namespace gib
