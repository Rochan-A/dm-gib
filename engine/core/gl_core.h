#pragma once

#include "util/macros.h"
#include "util/report/report.h"

#define GLAD_GL_IMPLEMENTATION
#include "third_party/glad/glad.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace gib {

// Class to initialize and manage core GLFW context.
class GLCore {
public:
  GLCore();
  ~GLCore();

  // Enable/Disable GLFW error logging.
  void ToggleGlfwErrorLogging(bool enable);

  // Enable/Disable OpenGL error logging.
  void ToggleOpenGlErrorLogging(bool enable);

  // Returns True if glfw is initialized.
  [[nodiscard]] bool IsInit() const { return glfw_init_success_ == GLFW_TRUE; }

  DISALLOW_COPY_AND_ASSIGN(GLCore);

private:
  // True if glfw error logging is enabled.
  bool glfw_error_logging_enabled_{false};

  // True if logging OpenGL debug messages is enabled.
  bool gl_debug_logging_enabled_{false};

  // Whether GLFW is initialized successfully.
  int glfw_init_success_{GLFW_FALSE};
};

} // namespace gib
