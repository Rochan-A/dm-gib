#pragma once

#include "util/macros.h"

#include "engine/core/frame_util.h"
#include "engine/core/gl_core.h"
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
// Handles Glfw window, its properties, and inputs.
class GlfwWindow {
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
  };

public:
  explicit GlfwWindow(std::shared_ptr<GLCore> &core, const std::string title,
                      const int width = kDefaultWidth,
                      const int height = kDefaultHeight, const int samples = 0,
                      const float fps_report_dt = 5.0);

  ~GlfwWindow() = default;

  // Returns pointer to GLFW window.
  GLFWwindow *GetGlfwWindowPtr();

  const float GetAvgFps() const;

  void Tick(const FrameTick &frame_tick);

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

  const GlfwWindowContext &GetGlfwWindowContext() const { return ctx_; }

  void DebugUI();

  DISALLOW_COPY_AND_ASSIGN(GlfwWindow);

private:
  void FramebufferSizeCallback(GLFWwindow *window, const int width,
                               const int height);

  const std::string title_;
  bool context_initialized_{false};
  FpsTracker fps_tracker_;

  GLFWwindow *glfw_window_ptr_{nullptr};
  GLFWmonitor *monitor_{nullptr};

  std::shared_ptr<GLCore> core_;

  GlfwWindowContext ctx_;
};

} // namespace gib
