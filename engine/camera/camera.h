#pragma once

#include "engine/core/types.h"
#include "third_party/glm/fwd.hpp"
#include <algorithm>
#define GLAD_GL_IMPLEMENTATION
#include "third_party/glad/glad.h"

#include "third_party/glm/glm.hpp"
#include "third_party/glm/gtc/matrix_transform.hpp"
#include "third_party/glm/gtc/type_ptr.hpp"
#include "util/report/macros.h"

namespace gib {

// Euler‑angle limits
constexpr float kPitchMin = -89.0f;
constexpr float kPitchMax = 89.0f;
constexpr float kZoomMin = 45.0f;  // tele
constexpr float kZoomMax = 100.0f; // wide-angle

// Enum that maps to WASD + QE (or arrows).
enum class Directions : unsigned char {
  FORWARD,
  BACKWARD,
  LEFT,
  RIGHT,
  UP,
  DOWN
};

struct CameraContext {
  // Mouse sensitivity (deg/px)
  BoundedType<float> sensitivity{0.1f, 1.f, 0.01f};
  // World‑units/s
  BoundedType<float> velocity{2.f, 10.f, 0.01f};
};

class Camera {
public:
  Camera(glm::vec3 init_pos = {0.f, 0.f, 3.f},
         glm::vec3 init_up = {0.f, 1.f, 0.f}, float init_zoom = kZoomMax,
         float init_yaw = -90.f, float init_pitch = 0.f)
      : position_{init_pos}, world_up_{glm::normalize(init_up)}, yaw_{init_yaw},
        pitch_{init_pitch}, zoom_{std::clamp(init_zoom, kZoomMin, kZoomMax)} {
    UpdateVectors();
  }
  ~Camera() = default;

  [[nodiscard]] const glm::vec3 &Position() const noexcept { return position_; }
  [[nodiscard]] const glm::vec3 &Front() const noexcept { return front_; }
  [[nodiscard]] float Zoom() const noexcept { return zoom_; }
  [[nodiscard]] float Yaw() const noexcept { return yaw_; }
  [[nodiscard]] float Pitch() const noexcept { return pitch_; }
  [[nodiscard]] glm::mat4 GetViewMatrix() const noexcept {
    return glm::lookAt(position_, position_ + front_, up_);
  }

  void ProcessKeyboard(Directions direction, float dt_seconds) noexcept {
    const float displacement = ctx_.velocity.value * dt_seconds;
    DEBUG("Got direction {} dt_seconds {}", (int)direction, dt_seconds);
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
    UpdateVectors();
  }

  void ProcessMouseMovement(float x_offset_pixels, float y_offset_pixels,
                            bool constrain_pitch = true) noexcept {
    yaw_ += x_offset_pixels * ctx_.sensitivity.value;
    pitch_ += y_offset_pixels * ctx_.sensitivity.value;
    if (constrain_pitch) {
      pitch_ = std::clamp(pitch_, kPitchMin, kPitchMax);
    }
    UpdateVectors();
  }

  void ProcessMouseScroll(const float y_offset) noexcept {
    // GLFW: positive y_offset means scroll up (zoom in / narrower FOV)
    zoom_ -= y_offset;
    zoom_ = std::clamp(zoom_, kZoomMin, kZoomMax);
    UpdateVectors();
  }

  void Tick(const CameraContext &ctx) noexcept { ctx_ = ctx; }

private:
  // Re‑compute the orthonormal basis from yaw/pitch
  void UpdateVectors() noexcept {
    // Spherical to Cartesian (right‑handed, Y‑up)
    const float yaw_rad = glm::radians(yaw_);
    const float pitch_rad = glm::radians(pitch_);

    glm::vec3 front;
    front.x = cosf(yaw_rad) * cosf(pitch_rad);
    front.y = sinf(pitch_rad);
    front.z = sinf(yaw_rad) * cosf(pitch_rad);
    front_ = glm::normalize(front);

    right_ = glm::normalize(glm::cross(front_, world_up_));
    up_ = glm::normalize(glm::cross(right_, front_));
  }

  // Camera state
  glm::vec3 position_;
  glm::vec3 front_{0.f, 0.f, -1.f};
  glm::vec3 up_{0.f, 1.f, 0.f};
  glm::vec3 right_{1.f, 0.f, 0.f};
  glm::vec3 world_up_;

  CameraContext ctx_{};

  // Euler angles
  float yaw_;
  float pitch_;

  // Field‑of‑view (degrees)
  float zoom_;
};
} // namespace gib