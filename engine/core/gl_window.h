#pragma once

#include "util/macros.h"

#include "engine/core/core.h"
#include "engine/core/window_util.h"
#include "util/report/macros.h"

#include "engine/core/input.h"
#include "engine/core/types.h"
#include <memory>

namespace gib {

static constexpr const char *kStencilFuncNames[] = {"ALWAYS", "MATCHING",
                                                    "NOTMATCHING"};
static constexpr const char *kCullModeNames[] = {"BACK", "FRONT"};

enum StencilTestFunc {
  ALWAYS = GL_ALWAYS,
  MATCHING = GL_EQUAL,
  NOTMATCHING = GL_NOTEQUAL,
};

enum FaceCullSetting {
  FRONT = GL_FRONT,
  BACK = GL_BACK,
};

// Context for the Glfw window.
struct GlfwWindowContext {
  bool enable_vsync{false};
  // Windowed if false.
  bool fullscreen{false};
  bool enable_resize_updates{true};
  bool enable_wireframe{false};
  bool enable_depth_test{false};
  bool enable_stencil_test{false};
  StencilTestFunc stencil_test_func;
  bool enable_stencil_updates{false};
  bool enable_alpha_blending{false};
  bool enable_face_cull{false};
  FaceCullSetting face_cull_setting;
  bool enable_seamless_cubemap{false};

  void DebugUI() {
    if (ImGui::CollapsingHeader("OpenGL Window",
                                ImGuiTreeNodeFlags_DefaultOpen)) {

      ImGui::Checkbox("Enable VSync", &enable_vsync);

      ImGui::Checkbox("Fullscreen", &fullscreen);
      ImGui::Checkbox("Resize Updates", &enable_resize_updates);
      ImGui::Checkbox("Wireframe", &enable_wireframe);
      ImGui::Checkbox("Depth Test", &enable_depth_test);

      ImGui::Checkbox("Stencil Test", &enable_stencil_test);
      if (enable_stencil_test) {
        int func = static_cast<int>(stencil_test_func);
        if (ImGui::Combo("Func", &func, kStencilFuncNames,
                         IM_ARRAYSIZE(kStencilFuncNames))) {
          stencil_test_func = static_cast<StencilTestFunc>(func);
        }
        ImGui::Checkbox("Write Stencil", &enable_stencil_updates);
      }

      ImGui::Checkbox("Alpha Blending", &enable_alpha_blending);

      ImGui::Checkbox("Face Cull", &enable_face_cull);
      if (enable_face_cull) {
        int mode = static_cast<int>(face_cull_setting);
        if (ImGui::Combo("Cull Mode", &mode, kCullModeNames,
                         IM_ARRAYSIZE(kCullModeNames))) {
          face_cull_setting = static_cast<FaceCullSetting>(mode);
        }
      }

      ImGui::Checkbox("Seamless Cubemap", &enable_seamless_cubemap);
    }
  }
};

// Handles Glfw window, its properties, and inputs.
class GlfwWindow {
public:
  explicit GlfwWindow(std::shared_ptr<GLCore> &core, const std::string title,
                      const int width = kDefaultWidth,
                      const int height = kDefaultHeight, const int samples = 0,
                      const float fps_report_dt = 5.0);

  ~GlfwWindow() = default;

  // Returns pointer to GLFW window.
  GLFWwindow *GetGlfwWindowPtr();

  const float GetAvgFps() const;

  void Tick(const FrameTick &frame_tick, const GlfwWindowContext &ctx);

  Size2D GetWindowSize();
  void SetWindowSize(const int width, const int height);

  void SetViewportSize(const int width, const int height);
  const float GetAspectRatio();

  void ToggleFullscreen(const bool &fullscreen);
  void ToggleResizeUpdates(const bool &enable_resize_updates);
  void ToggleVsync(const bool &enable_vsync);
  void ToggleWireframe(const bool &enable_wireframe);
  void ToggleDepthTest(const bool &enable_depth_test);
  void ToggleStencilTest(const bool &enable_stencil_test,
                         const StencilTestFunc &func);
  void ToggleStencilUpdates(const bool &enable_stencil_updates);
  void ToggleAlphaBlending(const bool &enable_alpha_blending);
  void ToggleFaceCull(const bool &enable_face_cull,
                      const FaceCullSetting &face_cull_setting);
  void ToggleSeamlessCubemap(const bool &enable_seamless_cubemap);

  GlfwWindowContext GetGlfwWindowContext() { return ctx_; }

  void DebugUI();

  FpsTracker fps_tracker;

  DISALLOW_COPY_AND_ASSIGN(GlfwWindow);

private:
  void FramebufferSizeCallback(GLFWwindow *window, const int width,
                               const int height);

  const std::string title_;
  bool context_initialized_{false};

  GlfwWindowContext ctx_;

  GLFWwindow *glfw_window_ptr_{nullptr};
  GLFWmonitor *monitor_{nullptr};

  std::shared_ptr<GLCore> core_;
};

} // namespace gib
