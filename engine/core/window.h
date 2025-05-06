#pragma once

#include "util/time/downsampler.h"
#include <chrono>
#include <optional>
#include <string>

#define GLAD_GL_IMPLEMENTATION
#include "third_party/glad/glad.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "engine/core/core.h"

static constexpr int kDefaultWidth = 800;
static constexpr int kDefaultHeight = 600;
static constexpr char const *kDefaultTitle = "dm_gib Engine";
static constexpr int kFrameDelta = 120;

// Controls behavior when Esc key is pressed.
enum class EscBehavior {
  NONE,
  TOGGLE_MOUSE_CAPTURE,
  CLOSE,
  // Uncaptures the mouse if it is captured, or else closes the app.
  UNCAPTURE_MOUSE_OR_CLOSE,
};

// Controls behavior when the LMB is pressed.
enum class MouseButtonBehavior {
  NONE,
  CAPTURE_MOUSE,
};

typedef struct WindowSize {
  int height;
  int width;
} WindowSize;

namespace core {

// Handles window and render loop.
class Window {
public:
  explicit Window(std::shared_ptr<GLCore> &core,
                  const char *title = kDefaultTitle,
                  const int width = kDefaultWidth,
                  const int height = kDefaultHeight, const int samples = 0);

  ~Window() = default;

  Window(Window &&other) noexcept;
  Window &operator=(Window &&other) noexcept;

  Window(const Window &) = delete;
  const Window &operator=(const Window &) = delete;

  // Set GLFW input mode.
  void SetGLFWInputMode(const int mode = GLFW_CURSOR,
                        const int value = GLFW_CURSOR_DISABLED);

  // Set GLFW key callback.
  void SetGLFWKeyCallback(GLFWkeyfun callback);

  void EnableVsync();
  void DisableVsync();

  void EnableWireframe();
  void DisableWireframe();

  // Returns pointer to GLFW window.
  GLFWwindow *GetWindow();

  const WindowSize GetWindowSize();
  void SetWindowSize(int width, int height);
  // Set viewport based on window size.
  void SetViewportSize();
  void MakeFullscreen();
  void MakeWindowed();
  const float GetAspectRatio();

  void EnableMouseCapture();
  void DisableMouseCapture();

  void EnableDepthTest();
  void EnableSeamlessCubemap();

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

  // Run game loop. Block until window close.
  void Run(std::function<void(float)> callback = {});

  const float GetAvgFps() const;

private:
  void ProcessInput(float deltaTime);

  void KeyCallback(int key, int scancode, int action, int mods);
  void ScrollCallback(double xoffset, double yoffset);
  void MouseMoveCallback(double xpos, double ypos);
  void MouseButtonCallback(int button, int action, int mods);
  void AddKeyPressHandler(int glfwKey, std::function<void(int)> handler);
  void AddMouseButtonHandler(int glfwMouseButton,
                             std::function<void(int)> handler);

  struct FpsTracker {
    std::chrono::steady_clock::time_point last_time_{
        std::chrono::steady_clock::now()};
    int frame_count_{1};
    float frame_delta_sum_{0.0f};
    float frame_delta_[kFrameDelta] = {0.0f};
    time_util::DownSampler downsampler_;

    FpsTracker(float report_dt) { downsampler_.SetDt(report_dt); }

    const float GetAvgFps() const {
      float avg_frame_delta =
          frame_delta_sum_ / std::min<int>(frame_count_, kFrameDelta);
      return 1.0f / avg_frame_delta;
    }

    float Tick() {
      std::chrono::steady_clock::time_point current_time =
          std::chrono::steady_clock::now();
      float delta_time_ =
          static_cast<float>(
              std::chrono::duration_cast<std::chrono::milliseconds>(
                  current_time - last_time_)
                  .count()) /
          1000.0f;
      last_time_ = current_time;

      unsigned int offset = frame_count_ % kFrameDelta;
      frame_delta_sum_ += (delta_time_ - frame_delta_[offset]);
      frame_delta_[offset] = delta_time_;

      if (downsampler_.UpdateAndCheckIfProcess(
              std::chrono::duration_cast<std::chrono::microseconds>(
                  current_time.time_since_epoch())
                  .count())) {
        INFO("Avg FPS: {} {}", GetAvgFps(),
             std::chrono::duration_cast<std::chrono::microseconds>(
                 current_time.time_since_epoch())
                 .count());
      }

      ++frame_count_;
      return delta_time_;
    }
  };

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
};

} // namespace core
