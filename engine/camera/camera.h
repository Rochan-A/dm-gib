#pragma once

#define GLAD_GL_IMPLEMENTATION
#include "third_party/glad/glad.h"

#include "third_party/glm/glm.hpp"
#include "third_party/glm/gtc/matrix_transform.hpp"
#include "third_party/glm/gtc/type_ptr.hpp"

namespace gib {

enum class Directions : unsigned char {
  FORWARD,
  BACKWARD,
  LEFT,
  RIGHT,
  UP,
  DOWN
};

class Camera {
public:
  Camera(glm::vec3 init_pos) : position_(init_pos) {}

  ~Camera() = default;

  float Zoom() { return zoom_; }

  glm::vec3 Position() { return position_; }

  void ProcessKeyboard(Directions direction, float dlt_time) {
    float dlt_dis = dlt_time * velocity_;
    switch (direction) {
    case Directions::FORWARD:
      position_ += dlt_dis * front_;
      break;
    case Directions::BACKWARD:
      position_ -= dlt_dis * front_;
      break;
    case Directions::LEFT:
      position_ -= glm::normalize(glm::cross(front_, up_)) * dlt_dis;
      break;
    case Directions::RIGHT:
      position_ += glm::normalize(glm::cross(front_, up_)) * dlt_dis;
      break;
    case Directions::UP:
      position_ += dlt_dis * up_;
      break;
    case Directions::DOWN:
      position_ -= dlt_dis * up_;
      break;
    default:
      break;
    }
  }

  void ProcessMouseMovement(float x_offset, float y_offset) {
    x_offset *= sensitivity_;
    y_offset *= sensitivity_;

    yaw_ += x_offset;
    pitch_ += y_offset;

    if (pitch_ > 89.0f)
      pitch_ = 89.0f;
    if (pitch_ < -89.0f)
      pitch_ = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw_)) * cos(glm::radians(pitch_));
    front.y = sin(glm::radians(pitch_));
    front.z = sin(glm::radians(yaw_)) * cos(glm::radians(pitch_));
    front_ = glm::normalize(front);
  }

  void ProcessMouseScroll(float y_offset) {
    if (zoom_ >= 1.0f && zoom_ <= 45.0f)
      if (zoom_ <= 1.0f)
        zoom_ -= y_offset;
    zoom_ = 1.0f;
    if (zoom_ >= 45.0f)
      zoom_ = 45.0f;
  }

  glm::mat4 GetViewMatrix() {
    return glm::lookAt(position_, position_ + front_, up_);
  }

private:
  glm::vec3 position_ = glm::vec3(0.0f, 0.0f, 3.0f);
  glm::vec3 front_ = glm::vec3(0.0f, 0.0f, -1.0f);
  glm::vec3 up_ = glm::vec3(0.0f, 1.0f, 0.0f);

  float velocity_ = 2.5f;
  float zoom_ = 45.0f;
  float sensitivity_ = 0.05f;
  float yaw_ = -90.0f;
  float pitch_ = 0.0f;
};
} // namespace gib