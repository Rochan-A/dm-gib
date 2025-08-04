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
  constexpr BoundedType(const ValueType value, const ValueType min,
                        const ValueType max)
      : value_(value), max_(max), min_(min) {}

  inline void Set(const ValueType &value) {
    value_ = std::clamp(value, min_, max_);
  }
  [[nodiscard]] inline ValueType Get() const { return value_; }
  [[nodiscard]] inline ValueType GetMin() const { return min_; }
  [[nodiscard]] inline ValueType GetMax() const { return max_; }

  BoundedType(const BoundedType &other)
      : value_(other.Get()), max_(other.GetMax()), min_(other.GetMin()) {}
  BoundedType &operator=(const BoundedType &other) {
    value_ = other.GetValue();
    max_ = other.GetMax();
    min_ = other.GetMin();
  }
  BoundedType(BoundedType &&other) = delete;
  BoundedType &operator=(BoundedType &&other) = delete;

private:
  ValueType value_;
  const ValueType max_;
  const ValueType min_;
};

} // namespace gib