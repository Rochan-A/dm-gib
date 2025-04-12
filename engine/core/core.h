#pragma once

#include <functional>
#include <memory>

#define GLAD_GL_IMPLEMENTATION
#include "third_party/glad/glad.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace core {

using gl_debug_callback_type = void(GLenum, GLenum, GLuint, GLenum, GLsizei,
                                    const GLchar *, const void *);

using glfw_error_callback_type = void(int, const char *);
using GlfwErrorCallback = std::function<glfw_error_callback_type>;

using GlDebugCallback = std::function<gl_debug_callback_type>;

inline void GlfwErrorPrintCallback(int error, const char *description) {
  fprintf(stderr, "GLFW ERROR: %s [error code %d]\n", description, error);
}

inline void GLAPIENTRY GlErrorPrintCallback(GLenum source, GLenum type,
                                            GLuint id, GLenum severity,
                                            GLsizei length,
                                            const GLchar *message,
                                            const void *user_param) {
  if (type == GL_DEBUG_TYPE_ERROR) {
    fprintf(stderr, "GL ERROR: %s [error type = 0x%x, severity = 0x%x]\n",
            message, type, severity);
  }
}

class GLCore {
public:
  explicit GLCore(GlDebugCallback gl_callback = {},
                  GlfwErrorCallback glfw_callback = {});

  [[nodiscard]] bool IsInit() const { return glfw_init_success_ == GLFW_TRUE; }

  void EnableGlfwErrorLogging();

  void DisableGlfwErrorLogging();

  void EnableGlErrorLogging();

  void DisableGlErrorLogging();

  static inline float GetTime() { return static_cast<float>(glfwGetTime()); }

  ~GLCore();

  GLCore(const GLCore &) = delete;
  const GLCore &operator=(const GLCore &) = delete;

private:
  // Whether or not to log glfw errors.
  bool glfw_error_logging_enabled_{true};

  // Whether or not to log OpenGL debug messages.
  bool gl_debug_logging_enabled_{true};

  int glfw_init_success_{GLFW_FALSE};

  GlfwErrorCallback glfw_error_callback_;

  GlDebugCallback gl_debug_callback_;
};

} // namespace core
