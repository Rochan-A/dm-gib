#include "engine/core/core.h"

namespace gib {

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

GLCore::GLCore() {
  glfw_init_success_ = glfwInit();
  ASSERT(glfw_init_success_ == GLFW_TRUE, "Failed to initialize GLFW!");
  INFO("Successfully initialized GLFW");
  EnableGlfwErrorLogging();
}

GLCore::~GLCore() {
  if (glfw_init_success_ == GLFW_TRUE) {
    glfwTerminate();
  } else {
    WARNING("GLFW failed to initialize, not calling glfwTerminate()");
  }
}

bool GLCore::EnableGlfwErrorLogging() {
  if (glfw_error_logging_enabled_) {
    return false;
  }
  glfwSetErrorCallback(&GlfwErrorPrintCallback);
  glfw_error_logging_enabled_ = true;
  INFO("Enabled GLFW error callback.");
  return true;
}

void GLCore::DisableGlfwErrorLogging() {
  glfwSetErrorCallback(nullptr);
  glfw_error_logging_enabled_ = false;
  INFO("Disabled OpenGL debug callback.");
}

bool GLCore::EnableGlErrorLogging() {
  if (!GLAD_GL_KHR_debug) {
    return false;
  }
  if (gl_debug_logging_enabled_) {
    return false;
  }
  glEnable(GL_DEBUG_OUTPUT);
  glDebugMessageCallback(&GlDebugPrintCallback, nullptr);
  gl_debug_logging_enabled_ = true;
  INFO("Enabled OpenGL debug callback.");
  return true;
}

void GLCore::DisableGlErrorLogging() {
  gl_debug_logging_enabled_ = false;
  glDisable(GL_DEBUG_OUTPUT);
  INFO("Disabled OpenGL debug callback.");
}

// GLCore::GLCore(GLCore &&other) noexcept
//     : glfw_error_logging_enabled_(other.glfw_error_logging_enabled_),
//       gl_debug_logging_enabled_(other.gl_debug_logging_enabled_),
//       glfw_init_success_(other.glfw_init_success_) {

//   other.glfw_error_logging_enabled_ = false;
//   other.gl_debug_logging_enabled_ = false;
//   other.glfw_init_success_ = GLFW_FALSE;
// }

// GLCore &GLCore::operator=(GLCore &&other) noexcept {
//   if (this != &other) {
//     glfw_error_logging_enabled_ = other.glfw_error_logging_enabled_;
//     gl_debug_logging_enabled_ = other.gl_debug_logging_enabled_;
//     glfw_init_success_ = other.glfw_init_success_;

//     other.glfw_error_logging_enabled_ = false;
//     other.gl_debug_logging_enabled_ = false;
//     other.glfw_init_success_ = GLFW_FALSE;
//   }
//   return *this;
// }

} // namespace gib