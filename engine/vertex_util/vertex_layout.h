#pragma once

#include <cstdint>
#include <vector>

#include <glad/glad.h>

namespace gib {

// Describes vertex params of an element.
struct VertexElement {
  GLuint location;
  GLint components;
  GLenum type;
  GLboolean normalized;
  // Bytes from vertex start
  std::size_t offset;
};

// Describes vertex layout as a collection of VertexElements.
struct VertexLayout {
  std::vector<VertexElement> elements;
  std::size_t stride = 0;
};

} // namespace gib