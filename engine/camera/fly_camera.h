#pragma once

#include "engine/camera/camera_base.h"
#include "engine/core/input.h"
#include "engine/core/types.h"
#include <limits>

namespace gib {

// Euler‑angle limits
static constexpr float kPitchMin = -89.0f;
static constexpr float kPitchMax = 89.0f;

static constexpr float kMinSensitivity = 0.01f;
static constexpr float kMaxSensitivity = 10.f;

static constexpr float kMinVelocity = 0.1f;
static constexpr float kMaxVelocity = 10.f;

// Fly camera model.
class FlyCameraModel final : public BaseCamera<FlyCameraModel> {
public:
  explicit FlyCameraModel(const glm::vec3 init_pos = {0.f, 0.f, 3.f},
                          const glm::vec3 init_up = {0.f, 1.f, 0.f},
                          const float init_zoom = kFovMax,
                          const float init_yaw = -90.f,
                          const float init_pitch = 0.f)
      : BaseCamera(init_pos, init_up, init_zoom, init_yaw, init_pitch, true) {}
  ~FlyCameraModel() = default;

  void DebugUIImpl() override {
    float speed = ctx_.velocity.Get();
    if (ImGui::SliderFloat("Speed (units/s)", &speed, ctx_.velocity.GetMin(),
                           ctx_.velocity.GetMax(), "%.2f")) {
      ctx_.velocity.Set(speed);
    }

    float sensitivity_ = ctx_.sensitivity.Get();
    if (ImGui::SliderFloat("Sensitivity (deg/pixel)", &sensitivity_,
                           ctx_.sensitivity.GetMin(), ctx_.sensitivity.GetMax(),
                           "%.2f")) {
      ctx_.sensitivity.Set(sensitivity_);
    }
  }

  void ProcessInputImpl(const Input &input,
                        const float &dt_seconds) noexcept override {
    const float displacement = ctx_.velocity.Get() * dt_seconds;
    if (displacement == 0.0f) {
      return;
    }

    if (input.key_state[GLFW_KEY_W] == KeyAction::PRESS ||
        input.key_state[GLFW_KEY_W] == KeyAction::REPEAT) {
      position_ += front_ * displacement;
    } else if (input.key_state[GLFW_KEY_S] == KeyAction::PRESS ||
               input.key_state[GLFW_KEY_S] == KeyAction::REPEAT) {
      position_ -= front_ * displacement;
    } else if (input.key_state[GLFW_KEY_A] == KeyAction::PRESS ||
               input.key_state[GLFW_KEY_A] == KeyAction::REPEAT) {
      position_ -= right_ * displacement;
    } else if (input.key_state[GLFW_KEY_D] == KeyAction::PRESS ||
               input.key_state[GLFW_KEY_D] == KeyAction::REPEAT) {
      position_ += right_ * displacement;
    } else if (input.key_state[GLFW_KEY_SPACE] == KeyAction::PRESS ||
               input.key_state[GLFW_KEY_SPACE] == KeyAction::REPEAT) {
      position_ += world_up_ * displacement;
    } else if (input.key_state[GLFW_KEY_LEFT_SHIFT] == KeyAction::PRESS ||
               input.key_state[GLFW_KEY_LEFT_SHIFT] == KeyAction::REPEAT) {
      position_ -= world_up_ * displacement;
    } else if (input.key_state[GLFW_KEY_Q] == KeyAction::PRESS ||
               input.key_state[GLFW_KEY_Q] == KeyAction::REPEAT) {
      position_ += world_up_ * displacement;
    } else if (input.key_state[GLFW_KEY_E] == KeyAction::PRESS ||
               input.key_state[GLFW_KEY_E] == KeyAction::REPEAT) {
      position_ -= world_up_ * displacement;
    }

    if (last_mouse_pos_.x == std::numeric_limits<float>::infinity()) {
      last_mouse_pos_ = input.mouse_pos;
    }
    const Offset delta = input.mouse_pos - last_mouse_pos_;
    yaw_ += delta.x * ctx_.sensitivity.Get();
    // Invert y
    pitch_ += -delta.y * ctx_.sensitivity.Get();
    // Pitch is constrained.
    pitch_ = std::clamp(pitch_, kPitchMin, kPitchMax);
    last_mouse_pos_ = input.mouse_pos;

    if (input.scroll_offset.y != 0.0f) {
      // GLFW: positive y_offset means scroll up (zoom in / narrower FOV)
      fov_.Set(fov_.Get() - input.scroll_offset.y);
    }
  }

  DISALLOW_COPY_AND_ASSIGN(FlyCameraModel);

private:
  Offset last_mouse_pos_{std::numeric_limits<float>::infinity(),
                         std::numeric_limits<float>::infinity()};

  struct FlyCameraContext {
    // Mouse sensitivity (deg/px)
    BoundedType<float> sensitivity{kMinSensitivity, kMaxSensitivity,
                                   kMinSensitivity};
    // World‑units/s
    BoundedType<float> velocity{kMinVelocity, kMaxVelocity, kMinVelocity};
  };

  FlyCameraContext ctx_;
};
} // namespace gib