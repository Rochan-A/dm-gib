#include <memory>
#include <thread>
#include <utility>

#define GLAD_GL_IMPLEMENTATION
#include "third_party/glad/glad.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "engine/core/core.h"
#include "util/report/macros.h"

namespace core {

GLCore::GLCore(GlDebugCallback gl_debug_callback,
               GLFWerrorfun glfw_error_callback)
    : glfw_error_callback_(std::move(glfw_error_callback)),
      gl_debug_callback_(std::move(gl_debug_callback)) {

  glfw_init_success_ = glfwInit();
  ASSERT(glfw_init_success_ == GLFW_TRUE, "Failed to initialize GLFW!");
  INFO("Successfully initialized GLFW");

  // Enable loggers if applicable.
  EnableGlfwErrorLogging();
  EnableGlErrorLogging();
}

GLCore::~GLCore() {
  ASSERT(glfw_init_success_ == GLFW_TRUE,
         "GLFW failed to initialize, not calling glfwTerminate()");
  glfwTerminate();
}

void GLCore::EnableGlfwErrorLogging() {
  if (!glfw_error_callback_) {
    return;
  }
  glfwSetErrorCallback(glfw_error_callback_);
  INFO("Set GLFW error callback.");
}

void GLCore::DisableGlfwErrorLogging() {
  glfw_error_logging_enabled_ = false;
  glfwSetErrorCallback(nullptr);
  INFO("Un-set OpenGL debug callback.");
}

void GLCore::EnableGlErrorLogging() {
  if (!gl_debug_callback_) {
    return;
  }
  glEnable(GL_DEBUG_OUTPUT);
  glDebugMessageCallback(gl_debug_callback_, nullptr);
  INFO("Set OpenGL debug callback.");
}

void GLCore::DisableGlErrorLogging() {
  gl_debug_logging_enabled_ = false;
  glDisable(GL_DEBUG_OUTPUT);
  INFO("Un-set OpenGL debug callback.");
}

// Move constructor
GLCore::GLCore(GLCore &&other) noexcept
    : glfw_error_logging_enabled_(other.glfw_error_logging_enabled_),
      gl_debug_logging_enabled_(other.gl_debug_logging_enabled_),
      glfw_init_success_(other.glfw_init_success_),
      glfw_error_callback_(std::move(other.glfw_error_callback_)),
      gl_debug_callback_(std::move(other.gl_debug_callback_)) {

  other.glfw_error_logging_enabled_ = true;
  other.gl_debug_logging_enabled_ = true;
  other.glfw_init_success_ = GLFW_FALSE;
}

} // namespace core