#pragma once

#include "util/macros.h"

#include "engine/core/frame_util.h"
#include "util/report/report.h"

#include "engine/core/input.h"
#include "engine/core/types.h"
#include <memory>

namespace gib {

static constexpr const char *kStencilFuncNames[] = {"ALWAYS", "MATCHING",
                                                    "NOTMATCHING"};
static constexpr const char *kFaceCullModeNames[] = {"BACK", "FRONT"};

enum StencilTestFunc : GLenum {
  ALWAYS = GL_ALWAYS,
  MATCHING = GL_EQUAL,
  NOTMATCHING = GL_NOTEQUAL,
};

enum FaceCullMode : GLenum {
  BACK = GL_BACK,
  FRONT = GL_FRONT,
};

// Context for the Glfw window.
// Handles Glfw window, its properties, and inputs.
class GlfwWindow {
  struct GlfwWindowContext {
    StencilTestFunc stencil_test_func;
    FaceCullMode face_cull_setting;

    bool enable_vsync{false};
    // Windowed if false.
    bool fullscreen{false};
    bool enable_resize_updates{true};
    bool enable_wireframe{false};
    bool enable_depth_test{false};
    bool enable_stencil_test{false};
    bool enable_stencil_updates{false};
    bool enable_alpha_blending{false};
    bool enable_face_cull{false};
    bool enable_seamless_cubemap{false};
  };

public:
  explicit GlfwWindow(std::string title,
                      Size2D size = {kDefaultWidth, kDefaultHeight},
                      int samples = 0, float fps_report_dt = 5.f);

  ~GlfwWindow();

  // Enable/Disable GLFW error logging.
  void ToggleGlfwErrorLogging(bool enable);

  // Enable/Disable OpenGL error logging.
  void ToggleOpenGlErrorLogging(bool enable);

  // Returns True if GFLW is initialized.
  [[nodiscard]] bool IsInit() const { return glfw_init_success_ == GLFW_TRUE; }

  // Returns pointer to GLFW window.
  [[nodiscard]] GLFWwindow *GetGlfwWindowPtr();

  // Returns average FPS.
  [[nodiscard]] const float GetAvgFps() const;

  void Tick(const FrameTick &frame_tick);

  [[nodiscard]] Size2D GetWindowSize();
  void SetWindowSize(const Size2D &size);

  void SetViewportSize(const Size2D &size);
  [[nodiscard]] float GetAspectRatio();

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
                      const FaceCullMode &face_cull_setting);
  void ToggleSeamlessCubemap(const bool &enable_seamless_cubemap);

  [[nodiscard]] const GlfwWindowContext &GetGlfwWindowContext() const {
    return ctx_;
  }

  void DebugUI();

  DISALLOW_COPY_AND_ASSIGN(GlfwWindow);

private:
  void FramebufferSizeCallback(GLFWwindow *window, int width, int height);

  const std::string title_;
  bool context_initialized_{false};
  FpsTracker fps_tracker_;

  GLFWwindow *glfw_window_ptr_{nullptr};
  GLFWmonitor *monitor_{nullptr};

  // True if glfw error logging is enabled.
  bool glfw_error_logging_enabled_{false};

  // True if logging OpenGL debug messages is enabled.
  bool gl_debug_logging_enabled_{false};

  // Whether GLFW is initialized successfully.
  int glfw_init_success_{GLFW_FALSE};

  GlfwWindowContext ctx_;
};

} // namespace gib
