#include "engine/core/window.h"
#include "engine/core/core.h"
#include <cstddef>
#include <utility>

namespace core {

Window::Window(GLCore &core, std::string title, int width, int height,
               bool fullscreen)
    : height_(height), width_(width), fullscreen_{fullscreen},
      title_(std::move(title)) {

  if (!core.IsInit()) {
    ASSERT();
  }

  if (fullscreen_) {
    monitor_ = glfwGetPrimaryMonitor();
    const GLFWvidmode *mode = glfwGetVideoMode(monitor_);

    glfwWindowHint(GLFW_RED_BITS, mode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

    width_ = mode->width;
    height_ = mode->height;
  }

  window_ =
      glfwCreateWindow(width_, height_, title_.c_str(), monitor_, nullptr);
  if (window_ != nullptr) {
    fprintf(stderr, "Failed to create GLFW window\n");
    exit(EXIT_FAILURE);
  }
}

void Window::SetGLFWInputMode(const int mode, const int value) {
  if (monitor_ == nullptr) {
    ASSERT();
  }
  glfwSetInputMode(window_, mode, value);
}

void Window::Init() {
  if (window_ == nullptr) {
    ASSERT();
  }
  glfwMakeContextCurrent(window_);
  context_initialized_ = true;

  glfwSetFramebufferSizeCallback(window_, resize_callback);
  glfwGetFramebufferSize(window_, &width_, &height_);
  resize_callback(window_, width_, height_);
}

void Window::SetGLFWSwapInterval(int interval) const {
  if (!context_initialized_) {
    ASSERT();
  }
  glfwSwapInterval(interval);
}

} // namespace core