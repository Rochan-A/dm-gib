#pragma once

#define GLAD_GL_IMPLEMENTATION
#include "third_party/glad/glad.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace imgui_util {

class ImGuiWindow {
public:
  ImGuiWindow(GLFWwindow *glfw_window_ptr, const bool multi_viewport = false);

  ImGuiWindow(ImGuiWindow &) = delete;
  ImGuiWindow(ImGuiWindow &&) = delete;
  ImGuiWindow &operator=(ImGuiWindow &&) = delete;
  ImGuiWindow &operator=(ImGuiWindow &) = delete;

  ~ImGuiWindow();

private:
  GLFWwindow *glfw_window_ptr_ = nullptr;
};

} // namespace imgui_util
