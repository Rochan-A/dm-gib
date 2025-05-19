#pragma once

#define GLAD_GL_IMPLEMENTATION
#include "third_party/glad/glad.h"

#include "util/report/macros.h"

#include "third_party/imgui/backends/imgui_impl_glfw.h"
#include "third_party/imgui/backends/imgui_impl_opengl3.h"
#include "third_party/imgui/imgui.h"

namespace imgui_util {

static const char *glsl_version = "#version 410";

// Imgui Window Manager.
class ImGuiWindow {
public:
  ImGuiWindow(GLFWwindow *glfw_window_ptr, const bool multi_viewport = false);

  ImGuiWindow(ImGuiWindow &) = delete;
  ImGuiWindow(ImGuiWindow &&) = delete;

  ImGuiWindow &operator=(ImGuiWindow &&) = delete;
  ImGuiWindow &operator=(ImGuiWindow &) = delete;

  ~ImGuiWindow();

private:
  GLFWwindow *glfw_window_ptr_{nullptr};
};

} // namespace imgui_util
