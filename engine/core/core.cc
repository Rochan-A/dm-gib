#include <memory>
#include <thread>
#include <utility>

#define GLAD_GL_IMPLEMENTATION
#include "third_party/glad/glad.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "engine/core/core.h"

namespace core {

GLCore::GLCore(GlDebugCallback gl_callback, GlfwErrorCallback glfw_callback)
    : glfw_error_callback_(std::move(glfw_callback)),
      gl_debug_callback_(std::move(gl_callback)) {

  if (glfw_error_callback_) {
    glfwSetErrorCallback(
        glfw_error_callback_.target<glfw_error_callback_type>());
  }

  glfw_init_success_ = glfwInit();

  // TODO(rochan): Do we need this? Move into window?
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
}

GLCore::~GLCore() {
  if (glfw_init_success_ == GLFW_TRUE) {
    glfwTerminate();
  }
}

void GLCore::EnableGlfwErrorLogging() { glfw_error_logging_enabled_ = true; }

void GLCore::DisableGlfwErrorLogging() { glfw_error_logging_enabled_ = false; }

void GLCore::EnableGlErrorLogging() {
  if (!gl_debug_callback_) {
    return;
  }
  gl_debug_logging_enabled_ = true;
  glEnable(GL_DEBUG_OUTPUT);
  glDebugMessageCallback(gl_debug_callback_.target<gl_debug_callback_type>(),
                         nullptr);
}

void GLCore::DisableGlErrorLogging() {
  gl_debug_logging_enabled_ = false;
  glDisable(GL_DEBUG_OUTPUT);
}

} // namespace core