#include "engine/core/window.h"
#include "util/report/macros.h"
#include "util/time/downsampler.h"
#include <chrono>
#include <cstddef>
#include <utility>

namespace core {

Window::Window(std::shared_ptr<GLCore> &core, const char *title,
               const int width, const int height, const int samples)
    : title_{title}, fps_tracker_(2.0), core_(core) {
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
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Required on Mac

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
  // EnableDepthTest();
  // EnableSeamlessCubemap();
  EnableResizeUpdates();
  EnableKeyInput();
  EnableScrollInput();
  EnableMouseMoveInput();
  EnableMouseButtonInput();
  INFO("Window setup complete.");
}

void Window::EnableVsync() {
  glfwSwapInterval(1);
  INFO("Enabled V-sync.");
}
void Window::DisableVsync() {
  glfwSwapInterval(0);
  INFO("Disabled V-sync.");
}

void Window::EnableWireframe() {
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  INFO("Enabled wireframe,");
}

void Window::DisableWireframe() {
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  INFO("Disabled wireframe.");
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

void Window::SetViewportSize() {
  const WindowSize size = GetWindowSize();
  glViewport(0, 0, size.width, size.height);
  INFO("Set viewport size to height: {}, width: {}.", size.height, size.width);
}

void Window::MakeFullscreen() {
  GLFWmonitor *monitor = glfwGetPrimaryMonitor();
  auto size = GetWindowSize();
  glfwSetWindowMonitor(window_, monitor, /* xpos */ 0,
                       /* unused ypos */ 0, size.width, size.height,
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
    self->SetWindowSize(width, height);
    glfwGetFramebufferSize(window, &width, &height);
    self->SetViewportSize();
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
    // auto self = static_cast<Window *>(glfwGetWindowUserPointer(window));
    //  self->KeyCallback(key, scancode, action, mods);
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
    // auto self = static_cast<Window *>(glfwGetWindowUserPointer(window));
    // self->ScrollCallback(xoffset, yoffset);
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
    // auto self = static_cast<Window *>(glfwGetWindowUserPointer(window));
    // self->MouseMoveCallback(xpos, ypos);
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
    // auto self = static_cast<Window *>(glfwGetWindowUserPointer(window));
    // self->MouseButtonCallback(button, action, mods);
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

// void Window::FramebufferSizeCallback(GLFWwindow *window, int width,
//                                      int height) {
//   glViewport(0, 0, width, height);

//   if (boundCamera_) {
//     boundCamera_->setAspectRatio(width / static_cast<float>(height));
//   }
//   if (boundCameraControls_) {
//     boundCameraControls_->resizeWindow(width, height);
//   }
// }

// void Window::ProcessInput(float deltaTime) {
//   if (boundCameraControls_) {
//     boundCameraControls_->processInput(window_, *boundCamera_, deltaTime);
//   }
// }

// void Window::keyCallback(int key, int scancode, int action, int mods) {
//   if (keyInputPaused_)
//     return;

//   if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
//     if (escBehavior_ == EscBehavior::TOGGLE_MOUSE_CAPTURE) {
//       auto inputMode = glfwGetInputMode(window_, GLFW_CURSOR);
//       if (inputMode == GLFW_CURSOR_NORMAL) {
//         enableMouseCapture();
//       } else {
//         disableMouseCapture();
//       }
//     } else if (escBehavior_ == EscBehavior::CLOSE) {
//       glfwSetWindowShouldClose(window_, true);
//     } else if (escBehavior_ == EscBehavior::UNCAPTURE_MOUSE_OR_CLOSE) {
//       auto inputMode = glfwGetInputMode(window_, GLFW_CURSOR);
//       if (inputMode == GLFW_CURSOR_DISABLED) {
//         disableMouseCapture();
//       } else {
//         // Close since mouse is not captured.
//         glfwSetWindowShouldClose(window_, true);
//       }
//     }
//   }

//   // Run handlers.
//   if (action == GLFW_PRESS) {
//     for (auto pair : keyPressHandlers_) {
//       int glfwKey;
//       std::function<void(int)> handler;
//       std::tie(glfwKey, handler) = pair;

//       if (key == glfwKey) {
//         handler(mods);
//       }
//     }
//   }
// }

// void Window::ScrollCallback(double xoffset, double yoffset) {
//   if (mouseInputPaused_)
//     return;

//   if (boundCameraControls_) {
//     boundCameraControls_->scroll(*boundCamera_, xoffset, yoffset,
//                                  mouseCaptured_);
//   }
// }

// void Window::MouseMoveCallback(double xpos, double ypos) {
//   if (mouseInputPaused_)
//     return;

//   if (boundCameraControls_) {
//     boundCameraControls_->mouseMove(*boundCamera_, xpos, ypos,
//     mouseCaptured_);
//   }
// }

// void Window::MouseButtonCallback(int button, int action, int mods) {
//   if (mouseInputPaused_)
//     return;

//   if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
//     if (mouseButtonBehavior_ == MouseButtonBehavior::CAPTURE_MOUSE) {
//       enableMouseCapture();
//     }
//   }

//   // Run handlers.
//   if (action == GLFW_PRESS) {
//     for (auto pair : mouseButtonHandlers_) {
//       int glfwMouseButton;
//       std::function<void(int)> handler;
//       std::tie(glfwMouseButton, handler) = pair;

//       if (button == glfwMouseButton) {
//         handler(mods);
//       }
//     }
//   }

//   if (boundCameraControls_) {
//     boundCameraControls_->mouseButton(*boundCamera_, button, action, mods,
//                                       mouseCaptured_);
//   }
// }

// void Window::addKeyPressHandler(int glfwKey, std::function<void(int)>
// handler) {
//   keyPressHandlers_.push_back(std::make_tuple(glfwKey, handler));
// }
// void Window::AddMouseButtonHandler(int glfwMouseButton,
//                                    std::function<void(int)> handler) {
//   mouseButtonHandlers_.push_back(std::make_tuple(glfwMouseButton, handler));
// }

// void Window::bindCamera(std::shared_ptr<Camera> camera) {
//   boundCamera_ = camera;
//   boundCamera_->setAspectRatio(getSize());
// }

// void Window::BindCameraControls(
//     std::shared_ptr<CameraControls> cameraControls) {
//   if (!boundCamera_) {
//     throw WindowException("ERROR::WINDOW::BIND_CAMERA_CONTROLS_FAILED\n"
//                           "Camera must be bound before camera controls.");
//   }
//   boundCameraControls_ = cameraControls;
//   ImageSize size = getSize();
//   boundCameraControls_->resizeWindow(size.width, size.height);
// }

const float Window::GetAvgFps() const { return fps_tracker_.GetAvgFps(); }

void Window::Run(std::function<void(float)> callback) {
  time_util::DownSampler fps_downsample(1.0f, true);

  while (!glfwWindowShouldClose(window_)) {
    float time_delta = fps_tracker_.Tick();

    // Clear the appropriate buffers.
    // glClearColor(clearColor_.r, clearColor_.g, clearColor_.b, clearColor_.a);
    // auto clearBits = GL_COLOR_BUFFER_BIT;
    // if (depthTestEnabled_) {
    //   clearBits |= GL_DEPTH_BUFFER_BIT;
    // }
    // if (stencilTestEnabled_) {
    //   clearBits |= GL_STENCIL_BUFFER_BIT;
    // }
    // glClear(clearBits);

    // Process necessary input.
    // processInput(delta_time_);

    // Call the loop function.
    callback(time_delta);

    // CheckForGlError();

    glfwSwapBuffers(window_);
    glfwPollEvents();
  }
}

void Window::SetGLFWInputMode(const int mode, const int value) {
  glfwSetInputMode(window_, mode, value);
  INFO("Set GLFW InputMode to mode: {}, value: {}", mode, value);
}

void Window::SetGLFWKeyCallback(GLFWkeyfun callback) {
  glfwSetKeyCallback(window_, callback);
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

} // namespace core