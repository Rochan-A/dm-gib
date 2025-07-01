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

  ToggleResizeUpdates(true);
  ToggleVsync(true);
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

void Window::SetWindowSize(const int width, const int height) {
  ASSERT(height > 0 && width > 0, "Width & height must be > 0, got ({}, {})",
         width, height);
  glfwSetWindowSize(glfw_window_ptr_, width, height);
  INFO("Set window size to height: {}, width: {}.", height, width);
}

void Window::SetViewportSize(const int width, const int height) {
  glViewport(0, 0, width, height);
}

void Window::ToggleFullscreen(const bool &fullscreen) {
  if (fullscreen == ctx_.fullscreen) {
    return;
  }

  if (fullscreen) {
    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode *mode = glfwGetVideoMode(monitor);
    glfwSetWindowMonitor(glfw_window_ptr_, monitor, /* xpos */ 0,
                         /* unused ypos */ 0, mode->width, mode->height,
                         /* refreshRate */ GLFW_DONT_CARE);
  } else {
    auto size = GetWindowSize();
    glfwSetWindowMonitor(glfw_window_ptr_, /* monitor */ nullptr, /* xpos */ 0,
                         /* ypos */ 0, size.width, size.height,
                         /* refreshRate */ GLFW_DONT_CARE);
  }
  ctx_.fullscreen = fullscreen;
  INFO("Window fullscreen: {}", fullscreen);
}

void Window::ToggleResizeUpdates(const bool &enable_resize_updates) {
  if (enable_resize_updates == ctx_.enable_resize_updates) {
    return;
  }
  if (enable_resize_updates) {
    auto callback = [](GLFWwindow *window, int width, int height) {
      auto self = static_cast<Window *>(glfwGetWindowUserPointer(window));
      self->FramebufferSizeCallback(window, width, height);
    };
    glfwSetFramebufferSizeCallback(glfw_window_ptr_, callback);
  } else {
    glfwSetFramebufferSizeCallback(glfw_window_ptr_, nullptr);
  }
  ctx_.enable_resize_updates = enable_resize_updates;
  INFO("Window resize update enabled: {}", enable_resize_updates);
}

void Window::ToggleVsync(const bool &enable_vsync) {
  if (enable_vsync == ctx_.enable_vsync) {
    return;
  }
  if (enable_vsync) {
    glfwSwapInterval(1);
  } else {
    glfwSwapInterval(0);
  }
  ctx_.enable_vsync = enable_vsync;
  INFO("Vsync enabled: {}", enable_vsync);
}

void Window::ToggleWireframe(const bool &enable_wireframe) {
  if (enable_wireframe == ctx_.enable_wireframe) {
    return;
  }
  if (enable_wireframe) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  } else {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  }
  ctx_.enable_wireframe = enable_wireframe;
  INFO("Wireframe enabled: {}", enable_wireframe);
}

void Window::ToggleDepthTest(const bool &enable_depth_test) {
  if (enable_depth_test == ctx_.enable_depth_test) {
    return;
  }
  if (enable_depth_test) {
    glEnable(GL_DEPTH_TEST);

  } else {
    glDisable(GL_DEPTH_TEST);
  }
  ctx_.enable_depth_test = enable_depth_test;
  INFO("Depth test enabled: {}", enable_depth_test);
}

void Window::ToggleStencilTest(const bool &enable_stencil_test,
                               const StencilTestFunc &func) {
  if (enable_stencil_test == ctx_.enable_stencil_test) {
    return;
  }
  if (enable_stencil_test) {
    glEnable(GL_STENCIL_TEST);
    // Only replace the value in the stencil buffer if both the stencil and
    // depth test pass.
    glStencilOp(/*sfail=*/GL_KEEP, /*dpfail=*/GL_KEEP, /*dppass=*/GL_REPLACE);
    // Set the stencil test to use the given `func` when comparing for
    // fragment liveness.
    glStencilFunc(func, /*ref=*/1, /*mask=*/0xFF);

  } else {
    glDisable(GL_STENCIL_TEST);
  }
  ctx_.enable_stencil_test = enable_stencil_test;
  INFO("Stencil test enabled: {}", enable_stencil_test);
}

void Window::ToggleStencilUpdates(const bool &enable_stencil_updates) {
  if (enable_stencil_updates == ctx_.enable_stencil_updates) {
    return;
  }
  if (enable_stencil_updates) {
    glStencilMask(0xFF);
  } else {
    glStencilMask(0x00);
  }
  ctx_.enable_stencil_updates = enable_stencil_updates;
  INFO("Stencil updates enabled: {}", enable_stencil_updates);
}

void Window::ToggleAlphaBlending(const bool &enable_alpha_blending) {
  if (enable_alpha_blending == ctx_.enable_alpha_blending) {
    return;
  }

  if (enable_alpha_blending) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);
  } else {
    glDisable(GL_BLEND);
  }
  ctx_.enable_alpha_blending = enable_alpha_blending;
  INFO("Alpha blending enabled: {}", enable_alpha_blending);
}

void Window::ToggleFaceCull(const bool &enable_face_cull,
                            const FaceCullSetting &face_cull_setting) {
  if (enable_face_cull == ctx_.enable_face_cull) {
    return;
  }

  if (enable_face_cull) {
    glEnable(GL_CULL_FACE);
    glCullFace(face_cull_setting);
  } else {
    glDisable(GL_CULL_FACE);
  }
  ctx_.enable_face_cull = enable_face_cull;
  INFO("Face culling enabled: {}", enable_face_cull);
}

void Window::ToggleSeamlessCubemap(const bool &enable_seamless_cubemap) {
  if (enable_seamless_cubemap == ctx_.enable_seamless_cubemap) {
    return;
  }

  if (enable_seamless_cubemap) {
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
  } else {
    glDisable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
  }
  ctx_.enable_seamless_cubemap = enable_seamless_cubemap;
  INFO("Seamless cubemap enabled: {}", enable_seamless_cubemap);
}

const float Window::GetAspectRatio() {
  const WindowSize size = GetWindowSize();
  return size.width / static_cast<float>(size.height);
}

void Window::FramebufferSizeCallback(GLFWwindow *window, const int width,
                                     const int height) {
  SetViewportSize(width, height);
}

void Window::Tick(const WindowTick &window_tick, const WindowContext &ctx) {
  fps_tracker.Tick(window_tick);
  ToggleVsync(ctx.enable_vsync);
  ToggleResizeUpdates(ctx.enable_resize_updates);
  ToggleWireframe(ctx.enable_wireframe);
  ToggleDepthTest(ctx.enable_depth_test);
  ToggleStencilTest(ctx.enable_stencil_test, ctx.stencil_test_func);
  ToggleStencilUpdates(ctx.enable_stencil_updates);
  ToggleAlphaBlending(ctx.enable_alpha_blending);
  ToggleFaceCull(ctx.enable_face_cull, ctx.face_cull_setting);
  ToggleSeamlessCubemap(ctx.enable_seamless_cubemap);
  ctx_ = ctx;
}

} // namespace gib