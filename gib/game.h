#pragma once

#include "engine/core/core.h"
#include "engine/core/window.h"
#include "util/imgui/imgui_window.h"
#include "util/time/time.h"
#include <functional>
#include <glm/vec4.hpp>
#include <memory>

const glm::vec4 kDefaultClearColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

namespace gib {

class Game {
public:
  explicit Game(std::string name);

  // Run game loop. Block until game ends.
  void Run();

  ~Game() = default;

private:
  std::shared_ptr<core::GLCore> core_;
  core::Window window_;

  imgui_util::ImGuiWindow imgui_window_;

  time_util::TimePoint last_time_{};
  glm::vec4 clear_color_ = kDefaultClearColor;
};

} // namespace gib
