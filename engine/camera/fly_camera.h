#pragma once

#include "engine/camera/camera_base.h"

namespace gib {

// Euler‑angle limits
static constexpr float kPitchMin = -89.0f;
static constexpr float kPitchMax = 89.0f;

// Fly camera model.
class FlyCameraModel final : public BaseCamera<FlyCameraModel> {
public:
  FlyCameraModel(const glm::vec3 init_pos = {0.f, 0.f, 3.f},
                 const glm::vec3 init_up = {0.f, 1.f, 0.f},
                 const float init_zoom = kFovMax, const float init_yaw = -90.f,
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

  void ProcessKeyboardImpl(const Directions &direction,
                           const float &dt_seconds) noexcept override {
    const float displacement = ctx_.velocity.Get() * dt_seconds;
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

  void
  ProcessMouseMovementImpl(const float &x_offset_pixels,
                           const float &y_offset_pixels) noexcept override {
    yaw_ += x_offset_pixels * ctx_.sensitivity.Get();
    pitch_ += y_offset_pixels * ctx_.sensitivity.Get();
    // Pitch is constrained.
    // TODO: Pull out into camera model param?
    pitch_ = std::clamp(pitch_, kPitchMin, kPitchMax);
  }

  void ProcessMouseScrollImpl(const float &y_offset) noexcept override {
    // GLFW: positive y_offset means scroll up (zoom in / narrower FOV)
    fov_.Set(fov_.Get() - y_offset);
  }

  DISALLOW_COPY_AND_ASSIGN(FlyCameraModel);

private:
  struct FlyCameraContext {
    // Mouse sensitivity (deg/px)
    BoundedType<float> sensitivity{0.1f, 1.f, 0.01f};
    // World‑units/s
    BoundedType<float> velocity{2.f, 10.f, 0.01f};
  };

  FlyCameraContext ctx_;
};
} // namespace gib