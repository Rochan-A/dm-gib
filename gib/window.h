#pragma once

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

// Owns OS window + GL context and drives Game object.
template <typename Derived> class GameWindow {
public:
  explicit GameWindow(const std::string name)
      : core_(std::make_shared<GLCore>()), window_(core_, name),
        input_manager_(window_.GetWindow(), MouseButtonBehavior::CAPTURE,
                       EscBehavior::UNCAPTURE_MOUSE_OR_CLOSE),
        imgui_window_(window_.GetWindow()) {
    input_manager_.EnableMouseButtonInput();
    input_manager_.EnableKeyInput();
    input_manager_.EnableScrollInput();
    input_manager_.EnableMouseMoveInput();
  }
  ~GameWindow() = default;

  // Enter the main loop. This call blocks until the user closes the window or
  // the application requests shutdown (glfwSetWindowShouldClose()).
  void Run() {
    bool enable_imgui = true;
    WindowContext window_ctx = window_.GetWindowContext();
    last_time_ = time_util::now();

    while (!glfwWindowShouldClose(window_.GetWindow())) {
      const WindowTick tick{time_util::now(),
                            time_util::elapsed_usec(last_time_)};
      last_time_ = tick.current_time;

      glfwPollEvents();
      window_.Tick(tick, window_ctx);

      glClearColor(clear_color_.r, clear_color_.g, clear_color_.b,
                   clear_color_.a);
      GLbitfield clear_bits = GL_COLOR_BUFFER_BIT;
      if (window_ctx.enable_depth_test) {
        clear_bits |= GL_DEPTH_BUFFER_BIT;
      }
      if (window_ctx.enable_stencil_test) {
        clear_bits |= GL_STENCIL_BUFFER_BIT;
      }
      glClear(clear_bits);

      static_cast<Derived *>(this)->Tick(tick, window_);

      if (enable_imgui) {
        window_ctx = window_.GetWindowContext();
        RenderImGui(window_ctx);
      }

      CHECK_GL_ERROR();
      glfwSwapBuffers(window_.GetWindow());

      static_cast<Derived *>(this)->Tock(tick, window_);
    }
  }

  DISALLOW_COPY_AND_ASSIGN(GameWindow);

private:
  std::shared_ptr<GLCore> core_;
  Window window_;
  time_util::TimePoint last_time_;
  InputManager input_manager_;

  void RenderImGui(WindowContext &window_ctx) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Engine‑level widgets
    if (ImGui::Begin("Debug")) {
      if (ImGui::CollapsingHeader("Performance",
                                  ImGuiTreeNodeFlags_DefaultOpen)) {
        char overlay[32];
        std::snprintf(overlay, sizeof(overlay), "Avg FPS %.02f",
                      window_.fps_tracker.GetAvgFps());
        ImGui::Checkbox("Enable VSync", &window_ctx.enable_vsync);
        ImGui::SameLine();
        ImGui::Text(
            "Frame Δt: %.3f ms",
            1e3f * window_.fps_tracker.fps_stat
                       .frame_deltas[window_.fps_tracker.fps_stat.frame_count]);
      }
      ImGui::Separator();
      static_cast<Derived *>(this)->DebugUI(window_);
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

  imgui_util::ImGuiWindow imgui_window_;
  glm::vec4 clear_color_ = kDefaultClearColor;
};
} // namespace gib
