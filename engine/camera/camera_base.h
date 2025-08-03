#pragma once

#include "engine/core/frame_util.h"
#include "engine/core/types.h"
#include "util/macros.h"
#include <algorithm>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "engine/core/input.h"
#include "third_party/imgui/imgui.h"

namespace gib {

// Euler‑angle limits
static constexpr float kFovMin = 45.0f;
static constexpr float kFovMax = 100.0f;

// Base camera. Derived class is responsible for implementing ProcessInputImpl()
// that updates internal state which influences the projection matrix.
template <typename CameraUpdateModel> class BaseCamera {
public:
  explicit BaseCamera(const glm::vec3 init_pos = {0.f, 0.f, 3.f},
                      const glm::vec3 init_up = {0.f, 1.f, 0.f},
                      const float init_fov = kFovMax,
                      const float init_yaw = -90.f,
                      const float init_pitch = 0.f,
                      const bool update_enabled = false)
      : position_{init_pos}, world_up_{glm::normalize(init_up)}, yaw_{init_yaw},
        pitch_{init_pitch}, update_enabled_(update_enabled) {
    fov_.Set(init_fov);
    static_cast<CameraUpdateModel *>(this)->UpdateVectors();
  }
  ~BaseCamera() = default;

  [[nodiscard]] const glm::vec3 &Position() const noexcept { return position_; }
  [[nodiscard]] const glm::vec3 &Front() const noexcept { return front_; }
  [[nodiscard]] float Fov() const noexcept { return fov_.Get(); }
  [[nodiscard]] float Yaw() const noexcept { return yaw_; }
  [[nodiscard]] float Pitch() const noexcept { return pitch_; }
  [[nodiscard]] glm::mat4 GetViewMatrix() const noexcept {
    return glm::lookAt(position_, position_ + front_, up_);
  }
  void ToggleEnableCameraUpdate(const bool &enable) noexcept {
    update_enabled_ = enable;
  }

  void ProcessInput(const Input &input, const float &dt_seconds) noexcept {
    PROFILE_SCOPE_N("camera::ProcessInput");
    if (!update_enabled_) {
      return;
    }
    auto *model_ptr = static_cast<CameraUpdateModel *>(this);
    model_ptr->ProcessInputImpl(input, dt_seconds);
    model_ptr->UpdateVectors();
  }

  virtual void Tick(const FrameTick &frame_tick) noexcept {};

  void DebugUI() {
    if (ImGui::CollapsingHeader("Camera Model",
                                ImGuiTreeNodeFlags_DefaultOpen)) {
      const glm::vec3 &position = Position();
      ImGui::Text("Pos: (%.2f, %.2f, %.2f)", position.x, position.y,
                  position.z);
      ImGui::Text("Yaw: %.1f°", Yaw());
      ImGui::Text("Pitch: %.1f°", Pitch());
      ImGui::Text("FOV: %.1f°", Fov());
      ImGui::Checkbox("Enable camera update?", &update_enabled_);
      ImGui::Separator();
      static_cast<CameraUpdateModel *>(this)->DebugUIImpl();
    }
  }

  virtual void DebugUIImpl(){};

  DISALLOW_COPY_AND_ASSIGN(BaseCamera);

protected:
  // Implemented by derived class.
  virtual void ProcessInputImpl(const Input &input,
                                const float &dt_seconds) noexcept {};

  // Camera state
  glm::vec3 position_;
  glm::vec3 front_{0.f, 0.f, -1.f};
  glm::vec3 up_{0.f, 1.f, 0.f};
  glm::vec3 right_{1.f, 0.f, 0.f};
  glm::vec3 world_up_;

  // Euler angles
  float yaw_;
  float pitch_;

  // Field‑of‑view (degrees)
  BoundedType<float> fov_{kFovMax, kFovMax, kFovMin};

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

// Static camera model.
class StaticCameraModel final : public BaseCamera<StaticCameraModel> {
public:
  explicit StaticCameraModel(const glm::vec3 init_pos = {0.f, 0.f, 3.f},
                             const glm::vec3 init_up = {0.f, 1.f, 0.f},
                             const float init_fov = kFovMax,
                             const float init_yaw = -90.f,
                             const float init_pitch = 0.f)
      : BaseCamera(init_pos, init_up, init_fov, init_yaw, init_pitch) {}
  ~StaticCameraModel() = delete;

  DISALLOW_COPY_AND_ASSIGN(StaticCameraModel);
};

} // namespace gib