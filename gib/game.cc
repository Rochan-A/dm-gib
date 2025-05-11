#include "gib/game.h"
#include "engine/core/core.h"
#include "engine/core/window.h"
#include "util/time/time.h"

#define GLAD_GL_IMPLEMENTATION
#include "third_party/glad/glad.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "util/report/error.h"
#include <functional>

#include "util/report/macros.h"

#include "third_party/imgui/backends/imgui_impl_glfw.h"
#include "third_party/imgui/backends/imgui_impl_opengl3.h"
#include "third_party/imgui/imgui.h"

namespace gib {

void ImGuiWindow_Tick(GLFWwindow *glfw_window_ptr) {
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  bool show_demo_window = true;
  ImGui::ShowDemoWindow(&show_demo_window);

  // Render frame
  ImGui::Render();
  int display_w, display_h;
  glfwGetFramebufferSize(glfw_window_ptr, &display_w, &display_h);
  glViewport(0, 0, display_w, display_h);
  glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

  // Update and Render additional Platform Windows
  // (Platform functions may change the current OpenGL context, so we
  // save/restore it to make it easier to paste this code elsewhere.
  GLFWwindow *backup_current_context = glfwGetCurrentContext();
  ImGui::UpdatePlatformWindows();
  ImGui::RenderPlatformWindowsDefault();
  glfwMakeContextCurrent(backup_current_context);
}

Game::Game(std::string name)
    : core_(std::make_shared<core::GLCore>()), window_(core_),
      imgui_window_(window_.GetWindow()) {}

void Game::Run() {

  bool enable_imgui_ = false;

  window_.AddKeyPressHandler(GLFW_KEY_F,
                             [this](int mods) { window_.MakeFullscreen(); });
  window_.AddKeyPressHandler(GLFW_KEY_G,
                             [this](int mods) { window_.MakeWindowed(); });

  window_.AddKeyPressHandler(GLFW_KEY_I, [&enable_imgui_](int mods) {
    if (enable_imgui_) {
      enable_imgui_ = false;
    } else {
      enable_imgui_ = true;
    }
  });

  while (!glfwWindowShouldClose(window_.GetWindow())) {
    core::WindowTick tick{time_util::now(),
                          time_util::elapsed_usec(last_time_)};

    window_.Tick(tick);
    glfwPollEvents();

    if (glfwGetWindowAttrib(window_.GetWindow(), GLFW_ICONIFIED) != 0) {
      return;
    }

    if (enable_imgui_)
      ImGuiWindow_Tick(window_.GetWindow());

    CHECK_GL_ERROR();

    glfwSwapBuffers(window_.GetWindow());

    if (glfwWindowShouldClose(window_.GetWindow())) {
      return;
    }
  }
}
} // namespace gib
