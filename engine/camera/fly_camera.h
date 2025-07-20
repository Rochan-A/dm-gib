#pragma once

#include "engine/camera/camera_base.h"

namespace gib {

// Euler‑angle limits
constexpr float kPitchMin = -89.0f;
constexpr float kPitchMax = 89.0f;

struct FlyCameraContext {
  // Mouse sensitivity (deg/px)
  BoundedType<float> sensitivity{0.1f, 1.f, 0.01f};
  // World‑units/s
  BoundedType<float> velocity{2.f, 10.f, 0.01f};

  void DebugUIImpl() {
    float speed = velocity.value;
    if (ImGui::SliderFloat("Speed (units/s)", &speed, velocity.lo, velocity.hi,
                           "%.2f")) {
      velocity.Set(speed);
    }

    float sensitivity_ = sensitivity.value;
    if (ImGui::SliderFloat("Sensitivity (deg/pixel)", &sensitivity_,
                           sensitivity.lo, sensitivity.hi, "%.3f")) {
      sensitivity.Set(sensitivity_);
    }
  }
};

// Fly camera model.
class FlyCameraModel final : BaseCamera<FlyCameraModel, FlyCameraContext> {
public:
  FlyCameraModel(const glm::vec3 init_pos = {0.f, 0.f, 3.f},
                 const glm::vec3 init_up = {0.f, 1.f, 0.f},
                 const float init_zoom = kZoomMax, const float init_yaw = -90.f,
                 const float init_pitch = 0.f)
      : BaseCamera(init_pos, init_up, init_zoom, init_yaw, init_pitch, true) {}
  ~FlyCameraModel() = default;

private:
  void ProcessKeyboardImpl(const Directions &direction,
                           const float &dt_seconds) noexcept {
    const float displacement = ctx_.velocity.value * dt_seconds;
    if (displacement == 0.0f) {
      return;
    }

    switch (direction) {
    case Directions::FORWARD:
      position_ += front_ * displacement;
      break;
    case Directions::BACKWARD:
      position_ -= front_ * displacement;
      break;
    case Directions::LEFT:
      position_ -= right_ * displacement;
      break;
    case Directions::RIGHT:
      position_ += right_ * displacement;
      break;
    case Directions::UP:
      position_ += world_up_ * displacement;
      break;
    case Directions::DOWN:
      position_ -= world_up_ * displacement;
      break;
    default:
      break;
    }
  }

  void ProcessMouseMovementImpl(const float &x_offset_pixels,
                                const float &y_offset_pixels,
                                bool constrain_pitch = true) noexcept {
    yaw_ += x_offset_pixels * ctx_.sensitivity.value;
    pitch_ += y_offset_pixels * ctx_.sensitivity.value;
    if (constrain_pitch) {
      pitch_ = std::clamp(pitch_, kPitchMin, kPitchMax);
    }
  }

  void ProcessMouseScrollImpl(const float &y_offset) noexcept {
    // GLFW: positive y_offset means scroll up (zoom in / narrower FOV)
    zoom_ -= y_offset;
    zoom_ = std::clamp(zoom_, kZoomMin, kZoomMax);
  }
};
} // namespace gib