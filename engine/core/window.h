#pragma once

#define GLAD_GL_IMPLEMENTATION
#include "third_party/glad/glad.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "engine/core/core.h"
#include "engine/core/window_util.h"
#include "util/report/macros.h"

#include "engine/core/types.h"

namespace gib {

// Handles Glfw window and its properties.
class Window {
public:
  explicit Window(std::shared_ptr<GLCore> &core, const std::string title,
                  const int width = kDefaultWidth,
                  const int height = kDefaultHeight, const int samples = 0,
                  const float fps_report_dt = 5.0);

  ~Window() = default;

  Window(Window &&other) = delete;
  Window &operator=(Window &&other) = delete;

  Window(const Window &) = delete;
  const Window &operator=(const Window &) = delete;

  // Set GLFW input mode.
  void SetGLFWInputMode(const int mode = GLFW_CURSOR,
                        const int value = GLFW_CURSOR_DISABLED);

  // Returns pointer to GLFW window.
  GLFWwindow *GetWindow();

  const float GetAvgFps() const;

  void Tick(const WindowTick &window_tick);

  const WindowSize GetWindowSize();
  void SetWindowSize(int width, int height);

  void SetViewportSize(int width, int height);

  void MakeFullscreen();
  void MakeWindowed();

  const float GetAspectRatio();

  void EnableResizeUpdates();
  void DisableResizeUpdates();

  void EnableVsync() { glfwSwapInterval(1); }
  void DisableVsync() { glfwSwapInterval(0); }

  void EnableWireframe() { glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); }
  void DisableWireframe() { glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); }

  void EnableDepthTest() { glEnable(GL_DEPTH_TEST); }
  void DisableDepthTest() { glDisable(GL_DEPTH_TEST); }

  void EnableStencilTest() {
    glEnable(GL_STENCIL_TEST);
    // Only replace the value in the stencil buffer if both the stencil and
    // depth test pass.
    glStencilOp(/*sfail=*/GL_KEEP, /*dpfail=*/GL_KEEP, /*dppass=*/GL_REPLACE);
  }
  void DisableStencilTest() { glDisable(GL_STENCIL_TEST); }

  void EnableStencilUpdates() { glStencilMask(0xFF); }
  void DisableStencilUpdates() { glStencilMask(0x00); }

  void StencilAlwaysDraw() { SetStencilFunc(GL_ALWAYS); }
  void StencilDrawWhenMatching() { SetStencilFunc(GL_EQUAL); }
  void StencilDrawWhenNotMatching() { SetStencilFunc(GL_NOTEQUAL); }
  void SetStencilFunc(GLenum func) {
    // Set the stencil test to use the given `func` when comparing for fragment
    // liveness.
    glStencilFunc(func, /*ref=*/1, /*mask=*/0xFF);
  }

  void EnableAlphaBlending() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);
  }
  void DisableAlphaBlending() { glDisable(GL_BLEND); }

  void EnableFaceCull() { glEnable(GL_CULL_FACE); }
  void DisableFaceCull() { glDisable(GL_CULL_FACE); }

  void EnableSeamlessCubemap() { glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS); }
  void DisableSeamlessCubemap() { glDisable(GL_TEXTURE_CUBE_MAP_SEAMLESS); }

  void CullFrontFaces() { glCullFace(GL_FRONT); }
  void CullBackFaces() { glCullFace(GL_BACK); }

  FpsTracker fps_tracker;

private:
  void FramebufferSizeCallback(GLFWwindow *window, int width, int height);

  bool fullscreen_{false};
  std::string title_;
  bool context_initialized_{false};

  bool resize_updates_enabled_{false};

  GLFWwindow *glfw_window_ptr_{nullptr};
  GLFWmonitor *monitor_{nullptr};

  std::shared_ptr<GLCore> core_;
};

} // namespace gib
