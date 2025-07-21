#pragma once

#include "third_party/glm/vec2.hpp"
#include <algorithm>

namespace gib {

typedef glm::vec<2, float> Offset;

// Wrapper around glm::vec2 with width & height accessor.
struct Size2D : glm::vec<2, int> {
  Size2D(const int width, const int height) {
    x = width;
    y = height;
  }
  const int &Width() const { return x; }
  const int &Height() const { return y; };
};

template <typename ValueType> class BoundedType {
public:
  constexpr BoundedType(ValueType value, ValueType max, ValueType min)
      : value_(value), max_(max), min_(min) {}
  void Set(const ValueType &value) { value_ = std::clamp(value, min_, max_); }
  const ValueType &Get() const { return value_; }
  const ValueType &GetMin() const { return min_; }
  const ValueType &GetMax() const { return max_; }

private:
  ValueType value_;
  ValueType max_;
  ValueType min_;
};

} // namespace gib