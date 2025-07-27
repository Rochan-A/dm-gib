#pragma once

#include <algorithm>
#include <glm/vec2.hpp>
#include <iostream>

namespace gib {

typedef glm::vec<2, float> Offset;

// Wrapper around glm::vec2 with width & height accessor.
struct Size2D : glm::vec<2, int> {
  Size2D(const int width, const int height) {
    x = width;
    y = height;
  }
  [[nodiscard]] const int &Width() const { return x; }
  [[nodiscard]] const int &Height() const { return y; };
};

inline std::ostream &operator<<(std::ostream &stream, const Size2D &size) {
  stream << "{w: " << size.Width() << ", h: " << size.Height() << "}";
  return stream;
}

template <typename ValueType> class BoundedType {
public:
  constexpr BoundedType(ValueType value, ValueType max, ValueType min)
      : value_(value), max_(max), min_(min) {}
  inline void Set(const ValueType &value) {
    value_ = std::clamp(value, min_, max_);
  }
  inline const ValueType &Get() const { return value_; }
  inline const ValueType &GetMin() const { return min_; }
  inline const ValueType &GetMax() const { return max_; }

private:
  ValueType value_;
  const ValueType max_;
  const ValueType min_;
};

} // namespace gib