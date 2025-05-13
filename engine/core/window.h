#pragma once

#include <chrono>
#include <optional>
#include <string>

#define GLAD_GL_IMPLEMENTATION
#include "third_party/glad/glad.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "engine/core/core.h"
#include "engine/core/input.h"
#include "util/report/macros.h"
#include "util/time/downsampler.h"
#include "util/time/time.h"

static constexpr int kDefaultWidth = 800;
static constexpr int kDefaultHeight = 600;
static constexpr char const *kDefaultTitle = "dm_gib Engine";
static constexpr int kFrameDelta = 120;

namespace core {

struct WindowSize {
  int height{0};
  int width{0};
};

struct WindowTick {
  WindowTick(time_util::TimePoint current_time,
             time_util::DurationUsec delta_time)
      : current_time(current_time), delta_time(delta_time) {}

  time_util::TimePoint current_time;
  time_util::DurationUsec delta_time;
};

// Track FPS over the last kFrameDelta frames.
struct FpsTracker {
  time_util::TimePoint last_time_{time_util::now()};
  int frame_count_{0}; // fixed
  time_util::DurationUsec frame_delta_sum_{time_util::DurationUsec{0}};
  time_util::DurationUsec frame_delta_[kFrameDelta] = {
      time_util::DurationUsec{0}};
  time_util::DownSampler downsampler_;

  explicit FpsTracker(float report_dt) { downsampler_.SetDt(report_dt); }

  // Average FPS over the sliding window.
  const float GetAvgFps() const {
    const int samples = std::min(frame_count_, kFrameDelta);
    if (samples == 0 || frame_delta_sum_.count() == 0) {
      return 0.f; // nothing recorded yet
    }
    return (1'000'000.f * samples) /
           static_cast<float>(frame_delta_sum_.count());
  }

  // Call once per rendered frame.
  void Tick(const WindowTick &window_tick) {
    const time_util::DurationUsec delta_time =
        time_util::elapsed_usec(last_time_, window_tick.current_time);
    last_time_ = window_tick.current_time;

    // Ring-buffer update.
    const unsigned int offset = frame_count_ % kFrameDelta;
    frame_delta_sum_ += delta_time - frame_delta_[offset];
    frame_delta_[offset] = delta_time;

    ++frame_count_;

    if (downsampler_.UpdateAndCheckIfProcess(window_tick.current_time)) {
      INFO("Avg FPS: {}", GetAvgFps());
    }
  }
};

// Handles Glfw window and its properties.
class Window {
public:
  explicit Window(std::shared_ptr<GLCore> &core,
                  const char *title = kDefaultTitle,
                  const int width = kDefaultWidth,
                  const int height = kDefaultHeight, const int samples = 0,
                  const float fps_report_dt = 5.0);

  ~Window() = default;

  Window(Window &&other) noexcept;
  Window &operator=(Window &&other) noexcept;

  Window(const Window &) = delete;
  const Window &operator=(const Window &) = delete;

  // Set GLFW input mode.
  void SetGLFWInputMode(const int mode = GLFW_CURSOR,
                        const int value = GLFW_CURSOR_DISABLED);

  // Returns pointer to GLFW window.
  GLFWwindow *GetWindow();

  const WindowSize GetWindowSize();
  void SetWindowSize(int width, int height);
  void SetViewportSize(int width, int height);
  void MakeFullscreen();
  void MakeWindowed();
  const float GetAspectRatio();

  // Window properties
  void EnableVsync() {
    glfwSwapInterval(1);
    INFO("Enabled V-sync.");
  }
  void DisableVsync() {
    glfwSwapInterval(0);
    INFO("Disabled V-sync.");
  }

  void EnableWireframe() {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    INFO("Enabled wireframe,");
  }

  void DisableWireframe() {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    INFO("Disabled wireframe.");
  }

  void EnableDepthTest() { glEnable(GL_DEPTH_TEST); }
  void DisableDepthTest() { glDisable(GL_DEPTH_TEST); }

  void EnableMouseCapture();
  void DisableMouseCapture();

  // TODO: Consider extracting stencil logic out to a separate class.
  void EnableStencilTest() {
    glEnable(GL_STENCIL_TEST);
    // Only replace the value in the stencil buffer if both the stencil and
    // depth test pass.
    glStencilOp(/*sfail=*/GL_KEEP, /*dpfail=*/GL_KEEP, /*dppass=*/GL_REPLACE);
    stencil_test_enabled_ = true;
  }
  void DisableStencilTest() {
    glDisable(GL_STENCIL_TEST);
    stencil_test_enabled_ = false;
  }

  void EnableStencilUpdates() { glStencilMask(0xFF); }
  void DisableStencilUpdates() { glStencilMask(0x00); }

  void StencilAlwaysDraw() { SetStencilFunc(GL_ALWAYS); }
  void StencilDrawWhenMatching() { SetStencilFunc(GL_EQUAL); }
  void StencilDrawWhenNotMatching() { SetStencilFunc(GL_NOTEQUAL); }
  void SetStencilFunc(GLenum func) {
    // Set the stencil test to use the given `func` when comparing for fragment
    // liveness.
    glStencilFunc(func, /*ref=*/1, /*mask=*/0xFF);
  }

  void EnableAlphaBlending() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);
  }
  void DisableAlphaBlending() { glDisable(GL_BLEND); }

  void EnableFaceCull() { glEnable(GL_CULL_FACE); }
  void DisableFaceCull() { glDisable(GL_CULL_FACE); }

  void EnableSeamlessCubemap() { glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS); }
  void DisableSeamlessCubemap() { glDisable(GL_TEXTURE_CUBE_MAP_SEAMLESS); }

  void CullFrontFaces() { glCullFace(GL_FRONT); }
  void CullBackFaces() { glCullFace(GL_BACK); }

  void EnableResizeUpdates();
  void DisableResizeUpdates();

  void EnableKeyInput();
  void DisableKeyInput();

  void EnableScrollInput();
  void DisableScrollInput();

  void EnableMouseMoveInput();
  void DisableMouseMoveInput();

  void EnableMouseButtonInput();
  void DisableMouseButtonInput();

  void AddKeyPressHandler(int glfwKey, std::function<void(int)> handler);
  void AddMouseButtonHandler(int glfwMouseButton,
                             std::function<void(int)> handler);

  const float GetAvgFps() const;

  void Tick(const WindowTick &window_tick);

private:
  void ProcessInput(float deltaTime);

  void KeyCallback(int key, int scancode, int action, int mods);
  void ScrollCallback(double xoffset, double yoffset);
  void MouseMoveCallback(double xpos, double ypos);
  void MouseButtonCallback(int button, int action, int mods);

  void FramebufferSizeCallback(GLFWwindow *window, int width, int height);

  bool fullscreen_{false};
  std::string title_;
  bool context_initialized_{false};

  FpsTracker fps_tracker_;

  GLFWwindow *window_{nullptr};
  GLFWmonitor *monitor_{nullptr};

  std::shared_ptr<GLCore> core_;

  bool mouse_captured_{false};
  bool resize_updates_enabled_{false};
  bool key_input_enabled_{false};
  bool scroll_input_enabled_{false};
  bool mouse_move_input_enabled_{false};
  bool mouse_button_input_enabled_{false};
  bool stencil_test_enabled_{false};
  bool mouse_input_paused_{false};
  bool key_input_paused_{false};

  MouseButtonBehavior mouse_button_behavior_{MouseButtonBehavior::NONE};
  EscBehavior esc_behavior_{EscBehavior::CLOSE};

  std::vector<std::tuple<int, std::function<void(int)>>> key_press_handlers_;
  std::vector<std::tuple<int, std::function<void(int)>>> mouse_button_handlers_;
};

} // namespace core
