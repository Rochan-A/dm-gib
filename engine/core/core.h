#pragma once

#include "util/report/macros.h"

#define GLAD_GL_IMPLEMENTATION
#include "third_party/glad/glad.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

std::string _GlErrorToString(GLenum error) {
  switch (error) {
  case GL_NO_ERROR:
    return "NO_ERROR";
  case GL_INVALID_ENUM:
    return "INVALID_ENUM";
  case GL_INVALID_VALUE:
    return "INVALID_VALUE";
  case GL_INVALID_OPERATION:
    return "INVALID_OPERATION";
  case GL_STACK_OVERFLOW:
    return "STACK_OVERFLOW";
  case GL_STACK_UNDERFLOW:
    return "STACK_UNDERFLOW";
  case GL_OUT_OF_MEMORY:
    return "OUT_OF_MEMORY";
  case GL_INVALID_FRAMEBUFFER_OPERATION:
    return "INVALID_FRAMEBUFFER_OPERATION";
  default:
    return "UNKNOWN";
  }
}

#define CHECK_GL_ERROR()                                                       \
  do {                                                                         \
    GLenum error = glGetError();                                               \
    ASSERT(error != GL_NO_ERROR, "ERROR::GL::{}", _GlErrorToString(error));    \
  } while (0)

namespace core {

// Class to initialize and manage core GLFW context.
class GLCore {
public:
  explicit GLCore();

  // Enable GLFW error logging if not enabled already.
  bool EnableGlfwErrorLogging();

  void DisableGlfwErrorLogging();

  // Enable OpenGL error logging if not enabled already.
  bool EnableGlErrorLogging();

  void DisableGlErrorLogging();

  bool IsInit() const { return glfw_init_success_ == GLFW_TRUE; }

  ~GLCore();

  GLCore(GLCore &&other) noexcept;

  GLCore &operator=(GLCore &&other) noexcept;

  GLCore(const GLCore &) = delete;
  const GLCore &operator=(const GLCore &) = delete;

private:
  // Whether or not to log glfw errors.
  bool glfw_error_logging_enabled_{false};

  // Whether or not to log OpenGL debug messages.
  bool gl_debug_logging_enabled_{false};

  // Whether GLFW is initialized successfully.
  int glfw_init_success_{GLFW_FALSE};
};

} // namespace core
