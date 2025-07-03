#pragma once

#include "engine/camera/camera.h"
#include "engine/core/core.h"
#include "engine/core/input.h"
#include "engine/core/window.h"
#include "engine/core/window_util.h"
#include "third_party/glm/matrix.hpp"
#include "util/imgui/imgui_util.h"
#include "util/imgui/imgui_window.h"
#include "util/macros.h"
#include "util/report/error.h"
#include "util/time/time.h"
#include <glm/vec4.hpp>
#include <memory>

#include "third_party/imgui/backends/imgui_impl_glfw.h"
#include "third_party/imgui/backends/imgui_impl_opengl3.h"
#include "third_party/imgui/imgui.h"

const glm::vec4 kDefaultClearColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

namespace gib {

// Context for the app window.
struct WindowContext {
  GlfwWindowContext glfw_window_ctx{};
  CameraContext camera_ctx{};

  bool enable_mouse_capture{false};
  bool enable_key_input{false};
  bool enable_scroll_input{false};
  bool enable_mouse_move_input{false};
  bool enable_mouse_button_input{false};
};

// Owns OS window and drives render loop.
template <typename WindowImpl> class BaseWindow {
public:
  explicit BaseWindow(const std::string name)
      : core_(std::make_shared<GLCore>()), window_(core_, name),
        imgui_window_(window_.GetGlfwWindowPtr()) {
    // Allow us to refer to this WindowImpl object while accessing C APIs.
    glfwSetWindowUserPointer(window_.GetGlfwWindowPtr(), this);
  }
  ~BaseWindow() = default;

  // Input related toggles.
  void ToggleMouseCapture(const bool &enable_mouse_capture);
  void ToggleKeyInput(const bool &enable_key_input);
  void ToggleScrollInput(const bool &enable_scroll_input);
  void ToggleMouseMoveInput(const bool &enable_mouse_move_input);
  void ToggleMouseButtonInput(const bool &enable_mouse_button_input);

  // Set ESC key and LMB behavior. TODO: Consider removing this.
  void SetEscKeyBehavior(const EscBehavior esc_behavior);
  void SetMouseButtonBehavior(const MouseButtonBehavior mouse_button_behavior);
  // Set GLFW input mode.
  void SetGLFWInputMode(const int mode = GLFW_CURSOR,
                        const int value = GLFW_CURSOR_DISABLED);

  Input GetInput() { return input_; }

  // Enter the main loop. This call blocks until the user closes the window or
  // the application requests shutdown (glfwSetWindowShouldClose()).
  void Run();

  DISALLOW_COPY_AND_ASSIGN(BaseWindow);

  WindowContext ctx_;

private:
  void RenderImGui(GlfwWindowContext &window_ctx);

  // Should be implemented by derived class.
  void KeyCallback(int key, int scancode, int action, int mods);
  void MouseButtonCallback(int button, int action, int mods);
  void ScrollCallback(double xoffset, double yoffset);
  void MouseMoveCallback(double xpos, double ypos);

  Input input_{};
  MouseButtonBehavior mouse_button_behavior_{};
  EscBehavior esc_behavior_{};

  std::shared_ptr<GLCore> core_;
  GlfwWindow window_;
  time_util::TimePoint last_time_;

  imgui_util::ImGuiWindow imgui_window_;
  glm::vec4 clear_color_ = kDefaultClearColor;
};

template <typename WindowImpl>
void BaseWindow<WindowImpl>::ToggleMouseCapture(
    const bool &enable_mouse_capture) {
  if (ctx_.enable_mouse_capture == enable_mouse_capture) {
    return;
  }
  if (enable_mouse_capture) {
    glfwSetInputMode(window_.GetGlfwWindowPtr(), GLFW_CURSOR,
                     GLFW_CURSOR_DISABLED);
  } else {
    glfwSetInputMode(window_.GetGlfwWindowPtr(), GLFW_CURSOR,
                     GLFW_CURSOR_NORMAL);
  }
  ctx_.enable_mouse_capture = enable_mouse_capture;
  DEBUG("Mouse capture enabled: {}", ctx_.enable_mouse_capture);
}

template <typename WindowImpl>
void BaseWindow<WindowImpl>::ToggleKeyInput(const bool &enable_key_input) {
  if (ctx_.enable_key_input == enable_key_input) {
    return;
  }
  if (enable_key_input) {
    auto callback = [](GLFWwindow *window, int key, int scancode, int action,
                       int mods) {
      ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
      // We use WantCaptureMouse here because it is true if the mouse is over
      // the Imgui window.
      // TODO: Fix this.
      ImGuiIO &io = ImGui::GetIO();
      if (!io.WantCaptureMouse ||
          !ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) ||
          !ImGui::IsAnyItemActive()) {
        auto self =
            reinterpret_cast<WindowImpl *>(glfwGetWindowUserPointer(window));
        self->KeyCallback(key, scancode, action, mods);
      }
    };
    glfwSetKeyCallback(window_.GetGlfwWindowPtr(), callback);
  } else {
    glfwSetKeyCallback(window_.GetGlfwWindowPtr(), nullptr);
  }
  ctx_.enable_key_input = enable_key_input;
  DEBUG("Window key input enabled {}", ctx_.enable_key_input);
}

template <typename WindowImpl>
void BaseWindow<WindowImpl>::ToggleScrollInput(
    const bool &enable_scroll_input) {
  if (ctx_.enable_scroll_input == enable_scroll_input) {
    return;
  }
  if (enable_scroll_input) {
    auto callback = [](GLFWwindow *window, double xoffset, double yoffset) {
      ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
      ImGuiIO &io = ImGui::GetIO();
      if (!io.WantCaptureMouse ||
          !ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) ||
          !ImGui::IsAnyItemActive()) {
        auto self =
            reinterpret_cast<WindowImpl *>(glfwGetWindowUserPointer(window));
        self->ScrollCallback(xoffset, yoffset);
      }
    };
    glfwSetScrollCallback(window_.GetGlfwWindowPtr(), callback);
  } else {
    glfwSetScrollCallback(window_.GetGlfwWindowPtr(), nullptr);
  }
  ctx_.enable_scroll_input = enable_scroll_input;
  DEBUG("Window scroll input enabled {}", enable_scroll_input);
}

template <typename WindowImpl>
void BaseWindow<WindowImpl>::ToggleMouseMoveInput(
    const bool &enable_mouse_move_input) {
  if (ctx_.enable_mouse_move_input == enable_mouse_move_input) {
    return;
  }
  if (enable_mouse_move_input) {
    auto callback = [](GLFWwindow *window, double xpos, double ypos) {
      ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);
      ImGuiIO &io = ImGui::GetIO();
      if (!io.WantSetMousePos ||
          !ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) ||
          !ImGui::IsAnyItemActive()) {
        auto self =
            reinterpret_cast<WindowImpl *>(glfwGetWindowUserPointer(window));
        self->MouseMoveCallback(xpos, ypos);
      }
    };
    glfwSetCursorPosCallback(window_.GetGlfwWindowPtr(), callback);
  } else {
    glfwSetCursorPosCallback(window_.GetGlfwWindowPtr(), nullptr);
  }
  ctx_.enable_mouse_move_input = enable_mouse_move_input;
  DEBUG("Window mouse move input enabled: {}", enable_mouse_move_input);
}

template <typename WindowImpl>
void BaseWindow<WindowImpl>::ToggleMouseButtonInput(
    const bool &enable_mouse_button_input) {
  if (ctx_.enable_mouse_button_input == enable_mouse_button_input) {
    return;
  }
  if (enable_mouse_button_input) {
    auto callback = [](GLFWwindow *window, int button, int action, int mods) {
      ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
      ImGuiIO &io = ImGui::GetIO();
      if (!io.WantCaptureMouse ||
          !ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) ||
          !ImGui::IsAnyItemActive()) {
        auto self =
            reinterpret_cast<WindowImpl *>(glfwGetWindowUserPointer(window));
        self->MouseButtonCallback(button, action, mods);
      }
    };
    glfwSetMouseButtonCallback(window_.GetGlfwWindowPtr(), callback);
  } else {
    glfwSetMouseButtonCallback(window_.GetGlfwWindowPtr(), nullptr);
  }
  ctx_.enable_mouse_button_input = enable_mouse_button_input;
  DEBUG("Window mouse button input enabled {}", enable_mouse_button_input);
}

template <typename WindowImpl>
void BaseWindow<WindowImpl>::KeyCallback(int key, int scancode, int action,
                                         int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    if (esc_behavior_ == EscBehavior::TOGGLE_MOUSE_CAPTURE) {
      ToggleMouseCapture(!ctx_.enable_mouse_capture);
    } else if (esc_behavior_ == EscBehavior::CLOSE) {
      glfwSetWindowShouldClose(window_.GetGlfwWindowPtr(), true);
    } else if (esc_behavior_ == EscBehavior::UNCAPTURE_MOUSE_OR_CLOSE) {
      auto inputMode =
          glfwGetInputMode(window_.GetGlfwWindowPtr(), GLFW_CURSOR);
      if (inputMode == GLFW_CURSOR_DISABLED) {
        ToggleMouseCapture(false);
      } else {
        // Close since mouse is not captured.
        glfwSetWindowShouldClose(window_.GetGlfwWindowPtr(), true);
      }
    }
  }
  input_.KeyCallback(key, scancode, action, mods);
}

template <typename WindowImpl>
void BaseWindow<WindowImpl>::ScrollCallback(double xoffset, double yoffset) {
  input_.ScrollCallback(xoffset, yoffset);
}

template <typename WindowImpl>
void BaseWindow<WindowImpl>::MouseMoveCallback(double xpos, double ypos) {
  input_.MouseMoveCallback(xpos, ypos);
}

template <typename WindowImpl>
void BaseWindow<WindowImpl>::MouseButtonCallback(int button, int action,
                                                 int mods) {
  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
    if (mouse_button_behavior_ == MouseButtonBehavior::CAPTURE) {
      ToggleMouseCapture(true);
    }
  }
  input_.MouseButtonCallback(button, action, mods);
}

template <typename WindowImpl>
void BaseWindow<WindowImpl>::SetEscKeyBehavior(const EscBehavior esc_behavior) {
  esc_behavior_ = esc_behavior;
}

template <typename WindowImpl>
void BaseWindow<WindowImpl>::SetMouseButtonBehavior(
    const MouseButtonBehavior mouse_button_behavior) {
  mouse_button_behavior_ = mouse_button_behavior;
}

template <typename WindowImpl>
void BaseWindow<WindowImpl>::SetGLFWInputMode(const int mode, const int value) {
  glfwSetInputMode(window_.GetGlfwWindowPtr(), mode, value);
  DEBUG("Set GLFW InputMode to mode: {}, value: {}", mode, value);
}

template <typename WindowImpl> void BaseWindow<WindowImpl>::Run() {
  bool enable_imgui = true;
  GlfwWindowContext window_ctx = window_.GetGlfwWindowContext();
  last_time_ = time_util::now();

  while (!glfwWindowShouldClose(window_.GetGlfwWindowPtr())) {
    const Tick tick{time_util::now(), time_util::elapsed_usec(last_time_)};
    last_time_ = tick.current_time;

    glfwPollEvents();
    window_.Tick(tick, window_ctx);

    glClearColor(clear_color_.r, clear_color_.g, clear_color_.b,
                 clear_color_.a);
    GLbitfield clear_bits = GL_COLOR_BUFFER_BIT;
    if (ctx_.glfw_window_ctx.enable_depth_test) {
      clear_bits |= GL_DEPTH_BUFFER_BIT;
    }
    if (ctx_.glfw_window_ctx.enable_stencil_test) {
      clear_bits |= GL_STENCIL_BUFFER_BIT;
    }
    glClear(clear_bits);

    static_cast<WindowImpl *>(this)->Tick(tick, window_);

    if (enable_imgui) {
      window_ctx = window_.GetGlfwWindowContext();
      RenderImGui(window_ctx);
    }

    CHECK_GL_ERROR();
    glfwSwapBuffers(window_.GetGlfwWindowPtr());

    static_cast<WindowImpl *>(this)->Tock(tick, window_);
  }
}

template <typename WindowImpl>
void BaseWindow<WindowImpl>::RenderImGui(GlfwWindowContext &window_ctx) {
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  // Engine‑level widgets
  if (ImGui::Begin("Debug")) {
    if (ImGui::CollapsingHeader("Performance",
                                ImGuiTreeNodeFlags_DefaultOpen)) {
      // Average FPS text
      ImGui::Text("Avg FPS: %.2f", window_.fps_tracker.GetAvgFps());
      // Current frame delta (safe lookup – fall back to 0)
      float frame_dt_s = 0.0f;
      const auto &stat = window_.fps_tracker.fps_stat;
      if (!stat.frame_deltas.empty()) {
        size_t idx = stat.frame_count % stat.frame_deltas.size();
        frame_dt_s = stat.frame_deltas[idx];
      }

      ImGui::SameLine();
      ImGui::Text("Frame dt: %.3f ms", 1e3f * frame_dt_s);

      ImGui::Checkbox("Enable VSync", &ctx_.glfw_window_ctx.enable_vsync);
    }

    if (ImGui::CollapsingHeader("Render State",
                                ImGuiTreeNodeFlags_DefaultOpen)) {
      ImGui::Checkbox("Fullscreen", &ctx_.glfw_window_ctx.fullscreen);
      ImGui::Checkbox("Resize Updates",
                      &ctx_.glfw_window_ctx.enable_resize_updates);
      ImGui::Checkbox("Wireframe", &ctx_.glfw_window_ctx.enable_wireframe);
      ImGui::Checkbox("Depth Test", &ctx_.glfw_window_ctx.enable_depth_test);

      if (ImGui::Checkbox("Stencil Test",
                          &ctx_.glfw_window_ctx.enable_stencil_test)) {
        // checkbox toggled; could apply GL state immediately if desired
      }
      if (ctx_.glfw_window_ctx.enable_stencil_test) {
        int func = static_cast<int>(ctx_.glfw_window_ctx.stencil_test_func);
        if (ImGui::Combo("Func", &func, kStencilFuncNames,
                         IM_ARRAYSIZE(kStencilFuncNames))) {
          ctx_.glfw_window_ctx.stencil_test_func =
              static_cast<StencilTestFunc>(func);
        }
        ImGui::Checkbox("Write Stencil",
                        &ctx_.glfw_window_ctx.enable_stencil_updates);
      }

      ImGui::Checkbox("Alpha Blending",
                      &ctx_.glfw_window_ctx.enable_alpha_blending);

      if (ImGui::Checkbox("Face Cull",
                          &ctx_.glfw_window_ctx.enable_face_cull)) {
        // toggle handled
      }
      if (ctx_.glfw_window_ctx.enable_face_cull) {
        int mode = static_cast<int>(ctx_.glfw_window_ctx.face_cull_setting);
        if (ImGui::Combo("Cull Mode", &mode, kCullModeNames,
                         IM_ARRAYSIZE(kCullModeNames))) {
          ctx_.glfw_window_ctx.face_cull_setting =
              static_cast<FaceCullSetting>(mode);
        }
      }

      ImGui::Checkbox("Seamless Cubemap",
                      &ctx_.glfw_window_ctx.enable_seamless_cubemap);
    }

    ImGui::Separator();
    static_cast<WindowImpl *>(this)->DebugUI(window_);
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
