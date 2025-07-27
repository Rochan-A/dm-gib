#include "engine/core/gl_window.h"
#include "engine/core/frame_util.h"

namespace gib {

GlfwWindow::GlfwWindow(std::shared_ptr<GLCore> &core, const std::string title,
                       const Size2D &size, const int samples,
                       const float fps_report_dt)
    : title_{title}, fps_tracker_(fps_report_dt), core_(core) {
  ASSERT(core->IsInit(),
         "Attempted to construct window for un-initialized Glfw!");
  ASSERT(size.Width() > 0 && size.Height() > 0,
         "Width & height must be > 0, got {}", size);

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
  glfw_window_ptr_ = glfwCreateWindow(size.Width(), size.Height(),
                                      title_.c_str(), nullptr, nullptr);
  ASSERT(glfw_window_ptr_ != nullptr, "GLFW window failed to initialize");

  glfwMakeContextCurrent(glfw_window_ptr_);

  gladLoadGL();
  core_->ToggleOpenGlErrorLogging(true);

  // Enable multisampling if needed.
  if (samples > 0) {
    glEnable(GL_MULTISAMPLE);
  }

  ToggleResizeUpdates(true);
  ToggleVsync(true);
  DEBUG("Window setup complete.");
}

GLFWwindow *GlfwWindow::GetGlfwWindowPtr() {
  ASSERT(glfw_window_ptr_ != nullptr, "GLFW window not constructed");
  return glfw_window_ptr_;
}

Size2D GlfwWindow::GetWindowSize() {
  Size2D size{0, 0};
  glfwGetWindowSize(glfw_window_ptr_, &size.x, &size.y);
  return size;
}

void GlfwWindow::SetWindowSize(const Size2D &size) {
  ASSERT(size.Width() > 0 && size.Height() > 0,
         "Width & height must be > 0, got {}", size);
  glfwSetWindowSize(glfw_window_ptr_, size.Width(), size.Height());
  DEBUG("Set window size to {}.", size);
}

void GlfwWindow::SetViewportSize(const Size2D &size) {
  glViewport(0, 0, size.Width(), size.Height());
}

void GlfwWindow::ToggleFullscreen(const bool &fullscreen) {
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
    glfwSetWindowMonitor(glfw_window_ptr_, /* monitor */ nullptr,
                         /* xpos */ 0,
                         /* ypos */ 0, size.Width(), size.Height(),
                         /* refreshRate */ GLFW_DONT_CARE);
  }
  ctx_.fullscreen = fullscreen;
  DEBUG("Window fullscreen: {}", fullscreen);
}

void GlfwWindow::ToggleResizeUpdates(const bool &enable_resize_updates) {
  if (enable_resize_updates == ctx_.enable_resize_updates) {
    return;
  }
  if (enable_resize_updates) {
    auto callback = [](GLFWwindow *window, int width, int height) {
      auto *self = static_cast<GlfwWindow *>(glfwGetWindowUserPointer(window));
      self->FramebufferSizeCallback(window, width, height);
    };
    glfwSetFramebufferSizeCallback(glfw_window_ptr_, callback);
  } else {
    glfwSetFramebufferSizeCallback(glfw_window_ptr_, nullptr);
  }
  ctx_.enable_resize_updates = enable_resize_updates;
  DEBUG("Window resize update enabled: {}", enable_resize_updates);
}

void GlfwWindow::ToggleVsync(const bool &enable_vsync) {
  if (enable_vsync == ctx_.enable_vsync) {
    return;
  }
  if (enable_vsync) {
    glfwSwapInterval(1);
  } else {
    glfwSwapInterval(0);
  }
  ctx_.enable_vsync = enable_vsync;
  DEBUG("Vsync enabled: {}", enable_vsync);
}

void GlfwWindow::ToggleWireframe(const bool &enable_wireframe) {
  if (enable_wireframe == ctx_.enable_wireframe) {
    return;
  }
  if (enable_wireframe) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  } else {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  }
  ctx_.enable_wireframe = enable_wireframe;
  DEBUG("Wireframe enabled: {}", enable_wireframe);
}

void GlfwWindow::ToggleDepthTest(const bool &enable_depth_test) {
  if (enable_depth_test == ctx_.enable_depth_test) {
    return;
  }
  if (enable_depth_test) {
    glEnable(GL_DEPTH_TEST);

  } else {
    glDisable(GL_DEPTH_TEST);
  }
  ctx_.enable_depth_test = enable_depth_test;
  DEBUG("Depth test enabled: {}", enable_depth_test);
}

void GlfwWindow::ToggleStencilTest(const bool &enable_stencil_test,
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
  DEBUG("Stencil test enabled: {}", enable_stencil_test);
}

void GlfwWindow::ToggleStencilUpdates(const bool &enable_stencil_updates) {
  if (enable_stencil_updates == ctx_.enable_stencil_updates) {
    return;
  }
  if (enable_stencil_updates) {
    glStencilMask(0xFF);
  } else {
    glStencilMask(0x00);
  }
  ctx_.enable_stencil_updates = enable_stencil_updates;
  DEBUG("Stencil updates enabled: {}", enable_stencil_updates);
}

void GlfwWindow::ToggleAlphaBlending(const bool &enable_alpha_blending) {
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
  DEBUG("Alpha blending enabled: {}", enable_alpha_blending);
}

void GlfwWindow::ToggleFaceCull(const bool &enable_face_cull,
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
  DEBUG("Face culling enabled: {}", enable_face_cull);
}

void GlfwWindow::ToggleSeamlessCubemap(const bool &enable_seamless_cubemap) {
  if (enable_seamless_cubemap == ctx_.enable_seamless_cubemap) {
    return;
  }

  if (enable_seamless_cubemap) {
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
  } else {
    glDisable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
  }
  ctx_.enable_seamless_cubemap = enable_seamless_cubemap;
  DEBUG("Seamless cubemap enabled: {}", enable_seamless_cubemap);
}

float GlfwWindow::GetAspectRatio() {
  Size2D const size = GetWindowSize();
  return static_cast<float>(size.Width()) / static_cast<float>(size.Height());
}

void GlfwWindow::FramebufferSizeCallback(GLFWwindow * /*window*/,
                                         const int width, const int height) {
  SetViewportSize(Size2D(width, height));
}

void GlfwWindow::DebugUI() {
  fps_tracker_.DebugUI();

  GlfwWindowContext ctx = ctx_;
  if (ImGui::CollapsingHeader("OpenGL Window",
                              ImGuiTreeNodeFlags_DefaultOpen)) {

    if (ImGui::Checkbox("Enable VSync", &ctx.enable_vsync)) {
      ToggleVsync(ctx.enable_vsync);
    }
    if (ImGui::Checkbox("Fullscreen", &ctx.fullscreen)) {
      ToggleFullscreen(ctx.fullscreen);
    }
    if (ImGui::Checkbox("Resize Updates", &ctx.enable_resize_updates)) {
      ToggleResizeUpdates(ctx.enable_resize_updates);
    }
    if (ImGui::Checkbox("Wireframe", &ctx.enable_wireframe)) {
      ToggleWireframe(ctx.enable_wireframe);
    }
    if (ImGui::Checkbox("Depth Test", &ctx.enable_depth_test)) {
      ToggleDepthTest(ctx.enable_depth_test);
    }
    if (ImGui::Checkbox("Stencil Test", &ctx.enable_stencil_test)) {
      ToggleStencilTest(ctx.enable_stencil_test, ctx.stencil_test_func);
      if (ctx_.enable_stencil_test) {
        int func = static_cast<int>(ctx.stencil_test_func);
        if (ImGui::Combo("Func", &func, kStencilFuncNames,
                         IM_ARRAYSIZE(kStencilFuncNames))) {
          ctx.stencil_test_func = static_cast<StencilTestFunc>(func);
        }
        if (ImGui::Checkbox("Write Stencil", &ctx.enable_stencil_updates)) {
          ToggleStencilUpdates(ctx.enable_stencil_updates);
        }
      }
    }
    if (ImGui::Checkbox("Alpha Blending", &ctx.enable_alpha_blending)) {
      ToggleAlphaBlending(ctx.enable_alpha_blending);
    }
    if (ImGui::Checkbox("Face Cull", &ctx.enable_face_cull)) {
      ToggleFaceCull(ctx.enable_face_cull, ctx.face_cull_setting);
      if (ctx.enable_face_cull) {
        int mode = static_cast<int>(ctx.face_cull_setting);
        if (ImGui::Combo("Cull Mode", &mode, kCullModeNames,
                         IM_ARRAYSIZE(kCullModeNames))) {
          ctx.face_cull_setting = static_cast<FaceCullSetting>(mode);
        }
      }
    }
    if (ImGui::Checkbox("Seamless Cubemap", &ctx.enable_seamless_cubemap)) {
      ToggleSeamlessCubemap(ctx.enable_seamless_cubemap);
    }
  }
}

void GlfwWindow::Tick(const FrameTick &frame_tick) {
  fps_tracker_.Tick(frame_tick);
}

} // namespace gib