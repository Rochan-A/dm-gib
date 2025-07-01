#pragma once

#include "util/macros.h"
#define GLAD_GL_IMPLEMENTATION
#include "third_party/glad/glad.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "engine/core/core.h"
#include "engine/core/window_util.h"
#include "util/report/macros.h"

#include "engine/core/types.h"

namespace gib {

enum StencilTestFunc {
  Always = GL_ALWAYS,
  Matching = GL_EQUAL,
  NotMatching = GL_NOTEQUAL,
};

enum FaceCullSetting {
  Front = GL_FRONT,
  Back = GL_BACK,
};

// State context of the Glfw window.
struct WindowContext {
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

// Handles Glfw window and its properties.
class Window {
public:
  explicit Window(std::shared_ptr<GLCore> &core, const std::string title,
                  const int width = kDefaultWidth,
                  const int height = kDefaultHeight, const int samples = 0,
                  const float fps_report_dt = 5.0);

  ~Window() = default;

  // Set GLFW input mode.
  void SetGLFWInputMode(const int mode = GLFW_CURSOR,
                        const int value = GLFW_CURSOR_DISABLED);

  // Returns pointer to GLFW window.
  GLFWwindow *GetWindow();

  const float GetAvgFps() const;

  void Tick(const WindowTick &window_tick, const WindowContext &ctx);

  const WindowSize GetWindowSize();
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

  WindowContext GetWindowContext() { return ctx_; }

  FpsTracker fps_tracker;

  DISALLOW_COPY_AND_ASSIGN(Window);

private:
  void FramebufferSizeCallback(GLFWwindow *window, const int width,
                               const int height);

  bool fullscreen_{false};
  const std::string title_;
  bool context_initialized_{false};

  WindowContext ctx_;

  GLFWwindow *glfw_window_ptr_{nullptr};
  GLFWmonitor *monitor_{nullptr};

  std::shared_ptr<GLCore> core_;
};

} // namespace gib
