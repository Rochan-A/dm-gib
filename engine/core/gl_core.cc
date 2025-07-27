#include "engine/core/gl_core.h"

namespace gib {

// Callback that prints warning for GLFW errors.
inline void GlfwErrorPrintCallback(int error, const char *description) {
  WARNING("GLFW ERROR: {} [error code {}]\n", description, error);
}

// Callback that prints warning for OpenGL errors.
inline void GLAPIENTRY GlDebugPrintCallback(GLenum /*source*/, GLenum type,
                                            GLuint /*id*/, GLenum severity,
                                            GLsizei /*length*/,
                                            const GLchar *message,
                                            const void * /*user_param*/) {
  if (type == GL_DEBUG_TYPE_ERROR) {
    WARNING("GL ERROR: {} [error type = 0x{}, severity = 0x{}]", message, type,
            severity);
  }
}

GLCore::GLCore() {
  glfw_init_success_ = glfwInit();
  ASSERT(glfw_init_success_ == GLFW_TRUE, "Failed to initialize GLFW!");
  DEBUG("Successfully initialized GLFW");
  ToggleGlfwErrorLogging(true);
}

GLCore::~GLCore() {
  if (glfw_init_success_ != GLFW_TRUE) {
    WARNING("GLFW failed to initialize, not calling glfwTerminate()");
    return;
  }
  glfwTerminate();
}

void GLCore::ToggleGlfwErrorLogging(const bool enable) {
  if (glfw_error_logging_enabled_ == enable) {
    return;
  }
  if (enable) {
    glfwSetErrorCallback(&GlfwErrorPrintCallback);
  } else {
    glfwSetErrorCallback(nullptr);
  }
  glfw_error_logging_enabled_ = enable;
  INFO("GLFW error callback logging enabled: {}", enable);
}

void GLCore::ToggleOpenGlErrorLogging(const bool enable) {
  if (GLAD_GL_KHR_debug == 0) {
    return;
  }
  if (gl_debug_logging_enabled_ == enable) {
    return;
  }
  if (enable) {
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(&GlDebugPrintCallback, nullptr);
  } else {
    glDisable(GL_DEBUG_OUTPUT);
  }
  gl_debug_logging_enabled_ = enable;
  INFO("OpenGL debug callback logging enabled: {}", enable);
}

} // namespace gib