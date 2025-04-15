#pragma once

#include "util/report/macros.h"
#include <functional>
#include <memory>

#define GLAD_GL_IMPLEMENTATION
#include "third_party/glad/glad.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace core {

using GlDebugCallback = GLDEBUGPROC;

// Callback that prints GLFW errors to stderr.
inline void GlfwErrorPrintCallback(int error, const char *description) {
  WARNING("GLFW ERROR: {} [error code {}]\n", description, error);
}

// Callback that prints OpenGL errors to stderr.
inline void GLAPIENTRY GlDebugPrintCallback(GLenum source, GLenum type,
                                            GLuint id, GLenum severity,
                                            GLsizei length,
                                            const GLchar *message,
                                            const void *user_param) {
  if (type == GL_DEBUG_TYPE_ERROR) {
    WARNING("GL ERROR: {} [error type = 0x{}, severity = 0x{}]", message, type,
            severity);
  }
}

// Class to initialize and manage core GLFW context.
class GLCore {
public:
  // If gl_callback or glfw_callback is set (default), set OpenGL and GLFW debug
  // callbacks, respectively.
  explicit GLCore(GlDebugCallback gl_debug_callback = GlDebugPrintCallback,
                  GLFWerrorfun glfw_error_callback = GlfwErrorPrintCallback);

  void EnableGlfwErrorLogging();

  void DisableGlfwErrorLogging();

  void EnableGlErrorLogging();

  void DisableGlErrorLogging();

  bool IsInit() const { return glfw_init_success_ == GLFW_TRUE; }

  inline float GetTime() { return static_cast<float>(glfwGetTime()); }

  ~GLCore();

  // Move constructor
  GLCore(GLCore &&other) noexcept;

  // Move assignment operator
  GLCore &operator=(GLCore &&other) noexcept {
    if (this != &other) {

      glfw_error_logging_enabled_ = other.glfw_error_logging_enabled_;
      gl_debug_logging_enabled_ = other.gl_debug_logging_enabled_;
      glfw_init_success_ = other.glfw_init_success_;
      glfw_error_callback_ = std::move(other.glfw_error_callback_);
      gl_debug_callback_ = std::move(other.gl_debug_callback_);

      other.glfw_error_logging_enabled_ = true;
      other.gl_debug_logging_enabled_ = true;
      other.glfw_init_success_ = GLFW_FALSE;
    }
    return *this;
  }

  // Disallow copy
  GLCore(const GLCore &) = delete;
  const GLCore &operator=(const GLCore &) = delete;

private:
  // Whether or not to log glfw errors.
  bool glfw_error_logging_enabled_{true};

  // Whether or not to log OpenGL debug messages.
  bool gl_debug_logging_enabled_{true};

  int glfw_init_success_{GLFW_FALSE};

  GLFWerrorfun glfw_error_callback_;

  GlDebugCallback gl_debug_callback_;
};

} // namespace core
