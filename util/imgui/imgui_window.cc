#include "util/imgui/imgui_window.h"
#include "util/report/macros.h"

#include "third_party/imgui/backends/imgui_impl_glfw.h"
#include "third_party/imgui/backends/imgui_impl_opengl3.h"
#include "third_party/imgui/imgui.h"

namespace imgui_util {

ImGuiWindow::ImGuiWindow(GLFWwindow *glfw_window_ptr, const bool multi_viewport)
    : glfw_window_ptr_(glfw_window_ptr) {

  // Dear ImGui Setup
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableKeyboard;           // Enable Keyboard Controls
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking
  if (multi_viewport) {
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport
  }
  ImGui::StyleColorsDark();
  ImGuiStyle &style = ImGui::GetStyle();
  style.WindowRounding = 0.0f;
  style.Colors[ImGuiCol_WindowBg].w = 1.0f;

  ImGui_ImplGlfw_InitForOpenGL(glfw_window_ptr_, true);
  const char *glsl_version = "#version 410";
  ImGui_ImplOpenGL3_Init(glsl_version);
  INFO("Done Imgui Setup!");
}

ImGuiWindow::~ImGuiWindow() {
  // Cleanup
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}

} // namespace imgui_util
