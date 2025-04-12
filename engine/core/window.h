#pragma once

#include <optional>
#include <string>

#define GLAD_GL_IMPLEMENTATION
#include "third_party/glad/glad.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "engine/core/core.h"

namespace core {

// Window resize callback function
inline void resize_callback(GLFWwindow * /*window*/, int width, int height) {
  glViewport(0, 0, width, height);
}

class Window {
public:
  explicit Window(GLCore &core, std::string title, int width = 640,
                  int height = 480, bool fullscreen = false);

  void SetGLFWInputMode(const int mode = GLFW_CURSOR,
                        const int value = GLFW_CURSOR_DISABLED);

  void Init();

  void SetGLFWSwapInterval(int interval) const;

  std::optional<GLFWwindow *> GetWindow() {
    return std::optional<GLFWwindow *>{window_};
  }

  ~Window() = default;

  Window(const Window &) = delete;
  const Window &operator=(const Window &) = delete;

private:
  int height_;
  int width_;

  bool fullscreen_{false};

  std::string title_;

  bool context_initialized_{false};

  GLFWwindow *window_{nullptr};
  GLFWmonitor *monitor_{nullptr};
};

} // namespace core
