#include "engine/core/window.h"
#include "util/time/downsampler.h"
#include <chrono>
#include <cstddef>
#include <utility>

namespace core {

Window::Window(std::shared_ptr<GLCore> &core, const char *title,
               const int width, const int height, const int samples,
               const float fps_report_dt)
    : title_{title}, fps_tracker_(fps_report_dt), core_(core) {
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
  window_ = glfwCreateWindow(width, height, title_.c_str(), nullptr, nullptr);
  ASSERT(window_ != nullptr, "GLFW window failed to initialize");

  glfwMakeContextCurrent(window_);

  gladLoadGL();
  core_->EnableGlErrorLogging();

  // Allow us to refer to the object while accessing C APIs.
  glfwSetWindowUserPointer(window_, this);

  // Enable multisampling if needed.
  if (samples > 0) {
    glEnable(GL_MULTISAMPLE);
  }

  // Enable a few options by default.
  EnableVsync();
  EnableDepthTest();
  EnableAlphaBlending();
  EnableSeamlessCubemap();
  EnableResizeUpdates();
  EnableKeyInput();
  EnableScrollInput();
  EnableMouseMoveInput();
  EnableMouseButtonInput();
  INFO("Window setup complete.");
}

Window::Window(Window &&other) noexcept
    : fullscreen_(other.fullscreen_), title_(std::move(other.title_)),
      context_initialized_(other.context_initialized_),
      fps_tracker_(other.fps_tracker_), window_(other.window_),
      monitor_(other.monitor_) {

  other.window_ = nullptr;
  other.monitor_ = nullptr;
  other.context_initialized_ = false;
}

Window &Window::operator=(Window &&other) noexcept {
  if (this != &other) {
    fullscreen_ = other.fullscreen_;
    title_ = std::move(other.title_);
    context_initialized_ = other.context_initialized_;
    window_ = other.window_;
    monitor_ = other.monitor_;
    fps_tracker_ = other.fps_tracker_;

    other.window_ = nullptr;
    other.monitor_ = nullptr;
    other.context_initialized_ = false;
  }
  return *this;
}

GLFWwindow *Window::GetWindow() {
  ASSERT(window_ != nullptr, "GLFW window not constructed");
  return window_;
}

const WindowSize Window::GetWindowSize() {
  WindowSize size;
  glfwGetWindowSize(window_, &size.width, &size.height);
  return size;
}

void Window::SetWindowSize(int width, int height) {
  ASSERT(height > 0 && width > 0, "Width & height must be > 0, got ({}, {})",
         width, height);
  glfwSetWindowSize(window_, width, height);
  INFO("Set window size to height: {}, width: {}.", height, width);
}

void Window::SetViewportSize(int width, int height) {
  glViewport(0, 0, width, height);
}

void Window::MakeFullscreen() {
  GLFWmonitor *monitor = glfwGetPrimaryMonitor();
  const GLFWvidmode *mode = glfwGetVideoMode(monitor);
  glfwSetWindowMonitor(window_, monitor, /* xpos */ 0,
                       /* unused ypos */ 0, mode->width, mode->height,
                       /* refreshRate */ GLFW_DONT_CARE);

  INFO("Set window size to fullscreen.");
}

void Window::MakeWindowed() {
  auto size = GetWindowSize();
  glfwSetWindowMonitor(window_, /* monitor */ nullptr, /* xpos */ 0,
                       /* ypos */ 0, size.width, size.height,
                       /* refreshRate */ GLFW_DONT_CARE);
}

void Window::EnableMouseCapture() {
  glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  mouse_captured_ = true;
  INFO("Enabled mouse capture.");
}

void Window::DisableMouseCapture() {
  glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
  mouse_captured_ = false;
  INFO("Disabled mouse capture.");
}

void Window::EnableResizeUpdates() {
  if (resize_updates_enabled_) {
    return;
  }
  auto callback = [](GLFWwindow *window, int width, int height) {
    auto self = static_cast<Window *>(glfwGetWindowUserPointer(window));
    self->FramebufferSizeCallback(window, width, height);
  };
  glfwSetFramebufferSizeCallback(window_, callback);
  resize_updates_enabled_ = true;
  INFO("Window resize updates enabled.");
}

void Window::DisableResizeUpdates() {
  if (!resize_updates_enabled_) {
    return;
  }
  glfwSetFramebufferSizeCallback(window_, nullptr);
  resize_updates_enabled_ = false;
  INFO("Disabled Window resize update.");
}

const float Window::GetAspectRatio() {
  const WindowSize size = GetWindowSize();
  return size.width / static_cast<float>(size.height);
}

void Window::EnableKeyInput() {
  if (key_input_enabled_) {
    return;
  }
  auto callback = [](GLFWwindow *window, int key, int scancode, int action,
                     int mods) {
    auto self = static_cast<Window *>(glfwGetWindowUserPointer(window));
    self->KeyCallback(key, scancode, action, mods);
  };
  glfwSetKeyCallback(window_, callback);
  key_input_enabled_ = true;
  INFO("Enabled Window key input.");
}

void Window::DisableKeyInput() {
  if (!key_input_enabled_) {
    return;
  }
  glfwSetKeyCallback(window_, nullptr);
  key_input_enabled_ = false;
  INFO("Disabled Window key input.");
}

void Window::EnableScrollInput() {
  if (scroll_input_enabled_) {
    return;
  }
  auto callback = [](GLFWwindow *window, double xoffset, double yoffset) {
    auto self = static_cast<Window *>(glfwGetWindowUserPointer(window));
    self->ScrollCallback(xoffset, yoffset);
  };
  glfwSetScrollCallback(window_, callback);
  scroll_input_enabled_ = true;
  INFO("Enabled Window scroll input.");
}

void Window::DisableScrollInput() {
  if (!scroll_input_enabled_) {
    return;
  }
  glfwSetScrollCallback(window_, nullptr);
  scroll_input_enabled_ = false;
  INFO("Disable Window scroll input.");
}

void Window::EnableMouseMoveInput() {
  if (mouse_move_input_enabled_) {
    return;
  }
  auto callback = [](GLFWwindow *window, double xpos, double ypos) {
    auto self = static_cast<Window *>(glfwGetWindowUserPointer(window));
    self->MouseMoveCallback(xpos, ypos);
  };
  glfwSetCursorPosCallback(window_, callback);
  mouse_move_input_enabled_ = true;
  INFO("Enable Window mouse move input.");
}

void Window::DisableMouseMoveInput() {
  if (!mouse_move_input_enabled_) {
    return;
  }
  glfwSetCursorPosCallback(window_, nullptr);
  mouse_move_input_enabled_ = false;
  INFO("Disable Window mouse move input.");
}

void Window::EnableMouseButtonInput() {
  if (mouse_button_input_enabled_) {
    return;
  }
  auto callback = [](GLFWwindow *window, int button, int action, int mods) {
    auto self = static_cast<Window *>(glfwGetWindowUserPointer(window));
    self->MouseButtonCallback(button, action, mods);
  };
  glfwSetMouseButtonCallback(window_, callback);
  mouse_button_input_enabled_ = true;
  INFO("Enable Window mouse button input.");
}

void Window::DisableMouseButtonInput() {
  if (!mouse_button_input_enabled_) {
    return;
  }
  glfwSetMouseButtonCallback(window_, nullptr);
  mouse_button_input_enabled_ = false;
  INFO("Disable Window mouse button input.");
}

void Window::FramebufferSizeCallback(GLFWwindow *window, int width,
                                     int height) {
  SetViewportSize(width, height);
}

void Window::KeyCallback(int key, int scancode, int action, int mods) {
  if (key_input_paused_)
    return;

  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    if (esc_behavior_ == EscBehavior::TOGGLE_MOUSE_CAPTURE) {
      auto inputMode = glfwGetInputMode(window_, GLFW_CURSOR);
      if (inputMode == GLFW_CURSOR_NORMAL) {
        EnableMouseCapture();
      } else {
        DisableMouseCapture();
      }
    } else if (esc_behavior_ == EscBehavior::CLOSE) {
      glfwSetWindowShouldClose(window_, true);
    } else if (esc_behavior_ == EscBehavior::UNCAPTURE_MOUSE_OR_CLOSE) {
      auto inputMode = glfwGetInputMode(window_, GLFW_CURSOR);
      if (inputMode == GLFW_CURSOR_DISABLED) {
        DisableMouseCapture();
      } else {
        // Close since mouse is not captured.
        glfwSetWindowShouldClose(window_, true);
      }
    }
  }

  // Run handlers.
  if (action == GLFW_PRESS) {
    for (auto pair : key_press_handlers_) {
      int glfwKey;
      std::function<void(int)> handler;
      std::tie(glfwKey, handler) = pair;

      if (key == glfwKey) {
        handler(mods);
      }
    }
  }
}

void Window::ScrollCallback(double xoffset, double yoffset) {
  if (mouse_input_paused_)
    return;
}

void Window::MouseMoveCallback(double xpos, double ypos) {
  if (mouse_input_paused_)
    return;
}

void Window::MouseButtonCallback(int button, int action, int mods) {
  if (mouse_input_paused_)
    return;

  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
    if (mouse_button_behavior_ == MouseButtonBehavior::CAPTURE_MOUSE) {
      EnableMouseCapture();
    }
  }

  // Run handlers.
  if (action == GLFW_PRESS) {
    for (auto pair : mouse_button_handlers_) {
      int glfwMouseButton;
      std::function<void(int)> handler;
      std::tie(glfwMouseButton, handler) = pair;

      if (button == glfwMouseButton) {
        handler(mods);
      }
    }
  }
}

void Window::AddKeyPressHandler(int glfwKey, std::function<void(int)> handler) {
  key_press_handlers_.push_back(std::make_tuple(glfwKey, handler));
}

void Window::AddMouseButtonHandler(int glfwMouseButton,
                                   std::function<void(int)> handler) {
  mouse_button_handlers_.push_back(std::make_tuple(glfwMouseButton, handler));
}

void Window::SetGLFWInputMode(const int mode, const int value) {
  glfwSetInputMode(window_, mode, value);
  INFO("Set GLFW InputMode to mode: {}, value: {}", mode, value);
}

void Window::Tick(const WindowTick &window_tick) {
  fps_tracker_.Tick(window_tick);
}

} // namespace core