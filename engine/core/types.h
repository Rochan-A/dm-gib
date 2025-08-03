#pragma once

#include <algorithm>
#include <fmt/format.h>
#include <glm/vec2.hpp>
#include <iostream>

namespace gib {

typedef glm::vec<2, float> Offset;

// Wrapper around glm::vec2 with Width() & Height() accessor.
struct Size2D : glm::vec<2, int> {
  Size2D(const int width, const int height) {
    x = width;
    y = height;
  }
  [[nodiscard]] const int &Width() const { return x; }
  [[nodiscard]] const int &Height() const { return y; };
};

inline std::string to_string(const Size2D &size) {
  return fmt::format("Size2D(width={}, height={})", size.Width(),
                     size.Height());
}

// Type that holds a value that conforms to a pre-determined minimum and maximum
// value.
template <typename ValueType> class BoundedType {
public:
  constexpr BoundedType(const ValueType value, const ValueType max,
                        const ValueType min)
      : value_(value), max_(max), min_(min) {}

  inline void Set(const ValueType &value) {
    value_ = std::clamp(value, min_, max_);
  }
  [[nodiscard]] inline const ValueType &Get() const { return value_; }
  [[nodiscard]] inline const ValueType &GetMin() const { return min_; }
  [[nodiscard]] inline const ValueType &GetMax() const { return max_; }

private:
  ValueType value_;
  const ValueType max_;
  const ValueType min_;
};

} // namespace gib