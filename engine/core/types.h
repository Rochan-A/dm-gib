#pragma once

#include <algorithm>
#include <glm/vec2.hpp>

namespace gib {

struct WindowSize {
  int height{0};
  int width{0};
};

typedef glm::vec2 Offset;

template <typename ValueType> struct BoundedType {
  ValueType value;
  ValueType hi;
  ValueType lo;
  constexpr BoundedType(ValueType v, ValueType max, ValueType min)
      : value(v), hi(max), lo(min) {}
  void Set(ValueType v) { value = std::clamp(v, lo, hi); }
};

} // namespace gib