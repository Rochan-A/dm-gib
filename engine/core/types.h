#pragma once

#include "third_party/glm/glm.hpp"

namespace gib {

struct WindowSize {
  int height{0};
  int width{0};
};

struct Offset {
  float x{0.f};
  float y{0.f};
};

} // namespace gib