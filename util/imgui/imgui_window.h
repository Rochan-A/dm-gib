#pragma once

#define GLAD_GL_IMPLEMENTATION
#include "third_party/glad/glad.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "util/report/report.h"

#include "third_party/imgui/imgui.h"
#include "third_party/imgui/imgui_impl_glfw.h"
#include "third_party/imgui/imgui_impl_opengl3.h"
#include "util/macros.h"

namespace imgui_util {

static const char *glsl_version = "#version 410";

// Imgui Window Manager.
class ImGuiWindow {
public:
  explicit ImGuiWindow(GLFWwindow *glfw_window_ptr,
                       bool install_callbacks = false,
                       bool multi_viewport = false);
  ~ImGuiWindow();

  DISALLOW_COPY_AND_ASSIGN(ImGuiWindow);

private:
  GLFWwindow *glfw_window_ptr_{nullptr};
};

} // namespace imgui_util
