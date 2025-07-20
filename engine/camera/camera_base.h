#pragma once

#include "engine/core/types.h"
#include <algorithm>

#include "third_party/glm/fwd.hpp"
#include "third_party/glm/glm.hpp"
#include "third_party/glm/gtc/matrix_transform.hpp"
#include "third_party/glm/gtc/type_ptr.hpp"

#include "third_party/imgui/backends/imgui_impl_glfw.h"
#include "third_party/imgui/backends/imgui_impl_opengl3.h"
#include "third_party/imgui/imgui.h"

namespace gib {

// Euler‑angle limits
constexpr float kZoomMin = 45.0f;  // tele
constexpr float kZoomMax = 100.0f; // wide-angle

// Enum that maps to WASD + QE (or arrows).
// TODO: Generalize this to any key press?
enum class Directions : unsigned char {
  FORWARD,
  BACKWARD,
  LEFT,
  RIGHT,
  UP,
  DOWN
};

// Base camera. Defaults to static.
template <typename CameraUpdateModel, typename CameraContext> class BaseCamera {
public:
  BaseCamera(const glm::vec3 init_pos = {0.f, 0.f, 3.f},
             const glm::vec3 init_up = {0.f, 1.f, 0.f},
             const float init_zoom = kZoomMax, const float init_yaw = -90.f,
             const float init_pitch = 0.f, const bool update_enabled = false)
      : position_{init_pos}, world_up_{glm::normalize(init_up)}, yaw_{init_yaw},
        pitch_{init_pitch}, zoom_{std::clamp(init_zoom, kZoomMin, kZoomMax)},
        update_enabled_(update_enabled) {
    static_cast<CameraUpdateModel *>(this)->UpdateVectors();
  }
  ~BaseCamera() = default;

  [[nodiscard]] const glm::vec3 &Position() const noexcept { return position_; }
  [[nodiscard]] const glm::vec3 &Front() const noexcept { return front_; }
  [[nodiscard]] float Zoom() const noexcept { return zoom_; }
  [[nodiscard]] float Yaw() const noexcept { return yaw_; }
  [[nodiscard]] float Pitch() const noexcept { return pitch_; }
  [[nodiscard]] glm::mat4 GetViewMatrix() const noexcept {
    return glm::lookAt(position_, position_ + front_, up_);
  }
  void ToggleEnableCameraUpdate(const bool &enable) noexcept {
    update_enabled_ = enable;
  }

  void ProcessKeyboard(const Directions &direction,
                       const float &dt_seconds) noexcept {
    if (!update_enabled_) {
      return;
    }
    CameraUpdateModel *model_ptr = static_cast<CameraUpdateModel *>(this);
    model_ptr->ProcessKeyboardImpl(direction, dt_seconds);
    model_ptr->UpdateVectors();
  }

  void ProcessMouseMovement(const float &x_offset_pixels,
                            const float &y_offset_pixels,
                            const bool constrain_pitch = true) noexcept {
    if (!update_enabled_) {
      return;
    }
    CameraUpdateModel *model_ptr = static_cast<CameraUpdateModel *>(this);
    model_ptr->ProcessMouseMovementImpl(x_offset_pixels, y_offset_pixels,
                                        constrain_pitch);
    model_ptr->UpdateVectors();
  }

  void ProcessMouseScroll(const float &y_offset) noexcept {
    if (!update_enabled_) {
      return;
    }
    CameraUpdateModel *model_ptr = static_cast<CameraUpdateModel *>(this);
    model_ptr->ProcessMouseScrollImpl(y_offset);
    model_ptr->UpdateVectors();
  }

  virtual void Tick(const CameraContext &ctx) noexcept { ctx_ = ctx; }

  void DebugUI() {
    const glm::vec3 &p = Position();
    ImGui::Text("Pos: (%.2f, %.2f, %.2f)", p.x, p.y, p.z);
    ImGui::Text("Yaw: %.1f°", Yaw());
    ImGui::Text("Pitch: %.1f°", Pitch());
    ImGui::Text("Zoom: %.1f° FOV", Zoom());
    ImGui::Checkbox("Enable camera update?", update_enabled_);
    ImGui::Separator();
    static_cast<CameraUpdateModel *>(this)->ImplDebugUI();
  }

protected:
  // Implemented by derived class.
  virtual void ProcessKeyboardImpl(const Directions &direction,
                                   const float &dt_seconds) noexcept {};
  virtual void
  ProcessMouseMovementImpl(const float &x_offset_pixels,
                           const float &y_offset_pixels,
                           const bool constrain_pitch = true) noexcept {};
  virtual void ProcessMouseScrollImpl(const float &y_offset) noexcept {};
  virtual void DebugUIImpl(){};

  // Camera state
  glm::vec3 position_;
  glm::vec3 front_{0.f, 0.f, -1.f};
  glm::vec3 up_{0.f, 1.f, 0.f};
  glm::vec3 right_{1.f, 0.f, 0.f};
  glm::vec3 world_up_;

  CameraContext ctx_;

  // Euler angles
  float yaw_;
  float pitch_;

  // Field‑of‑view (degrees)
  float zoom_;

  bool update_enabled_;

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
};
} // namespace gib