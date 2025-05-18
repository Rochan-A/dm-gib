#include "engine/core/window.h"

namespace gib {

Window::Window(std::shared_ptr<GLCore> &core, const std::string title,
               const int width, const int height, const int samples,
               const float fps_report_dt)
    : fps_tracker(fps_report_dt), title_{title}, core_(core) {
  ASSERT(core->IsInit(),
         "Attempted to construct window for un-initialized core!");
  ASSERT(height > 0 && width > 0, "Width & height must be > 0, got ({}, {})",
         width, height);

  monitor_ = glfwGetPrimaryMonitor();
  const GLFWvidmode *mode = glfwGetVideoMode(monitor_);

  glfwWindowHint(GLFW_RED_BITS, mode->redBits);
  glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
  glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
  glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Required on Mac
#endif

  // Start in windowed mode.
  glfw_window_ptr_ =
      glfwCreateWindow(width, height, title_.c_str(), nullptr, nullptr);
  ASSERT(glfw_window_ptr_ != nullptr, "GLFW window failed to initialize");

  glfwMakeContextCurrent(glfw_window_ptr_);

  gladLoadGL();
  core_->EnableGlErrorLogging();

  // Allow us to refer to the object while accessing C APIs.
  glfwSetWindowUserPointer(glfw_window_ptr_, this);

  // Enable multisampling if needed.
  if (samples > 0) {
    glEnable(GL_MULTISAMPLE);
  }

  EnableResizeUpdates();
  INFO("Window setup complete.");
}

GLFWwindow *Window::GetWindow() {
  ASSERT(glfw_window_ptr_ != nullptr, "GLFW window not constructed");
  return glfw_window_ptr_;
}

const WindowSize Window::GetWindowSize() {
  WindowSize size;
  glfwGetWindowSize(glfw_window_ptr_, &size.width, &size.height);
  return size;
}

void Window::SetWindowSize(int width, int height) {
  ASSERT(height > 0 && width > 0, "Width & height must be > 0, got ({}, {})",
         width, height);
  glfwSetWindowSize(glfw_window_ptr_, width, height);
  INFO("Set window size to height: {}, width: {}.", height, width);
}

void Window::SetViewportSize(int width, int height) {
  glViewport(0, 0, width, height);
}

void Window::MakeFullscreen() {
  GLFWmonitor *monitor = glfwGetPrimaryMonitor();
  const GLFWvidmode *mode = glfwGetVideoMode(monitor);
  glfwSetWindowMonitor(glfw_window_ptr_, monitor, /* xpos */ 0,
                       /* unused ypos */ 0, mode->width, mode->height,
                       /* refreshRate */ GLFW_DONT_CARE);

  INFO("Set window size to fullscreen.");
}

void Window::MakeWindowed() {
  auto size = GetWindowSize();
  glfwSetWindowMonitor(glfw_window_ptr_, /* monitor */ nullptr, /* xpos */ 0,
                       /* ypos */ 0, size.width, size.height,
                       /* refreshRate */ GLFW_DONT_CARE);
}

void Window::EnableResizeUpdates() {
  if (resize_updates_enabled_) {
    return;
  }
  auto callback = [](GLFWwindow *window, int width, int height) {
    auto self = static_cast<Window *>(glfwGetWindowUserPointer(window));
    self->FramebufferSizeCallback(window, width, height);
  };
  glfwSetFramebufferSizeCallback(glfw_window_ptr_, callback);
  resize_updates_enabled_ = true;
  INFO("Window resize updates enabled.");
}

void Window::DisableResizeUpdates() {
  if (!resize_updates_enabled_) {
    return;
  }
  glfwSetFramebufferSizeCallback(glfw_window_ptr_, nullptr);
  resize_updates_enabled_ = false;
  INFO("Disabled Window resize update.");
}

const float Window::GetAspectRatio() {
  const WindowSize size = GetWindowSize();
  return size.width / static_cast<float>(size.height);
}

void Window::FramebufferSizeCallback(GLFWwindow *window, int width,
                                     int height) {
  SetViewportSize(width, height);
}

void Window::Tick(const WindowTick &window_tick) {
  fps_tracker.Tick(window_tick);
}

} // namespace gib