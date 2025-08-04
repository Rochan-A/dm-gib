#include "engine/vertex_util/vertex_array.h"

namespace gib {

VertexArray::VertexArray() {
  glGenVertexArrays(1, &vao_);
  glGenBuffers(1, &vbo_);
  glGenBuffers(1, &ebo_);
}

VertexArray::~VertexArray() {
  if (ebo_ != 0u) {
    glDeleteBuffers(1, &ebo_);
  }
  if (vbo_ != 0u) {
    glDeleteBuffers(1, &vbo_);
  }
  if (vao_ != 0u) {
    glDeleteVertexArrays(1, &vao_);
  }
}

void VertexArray::SetVertexData(const void *data, const std::size_t size,
                                const GLenum usage) const {
  ASSERT(data && size,
         "Index data ({}) and size ({}) must be non-null & non-zero!", data,
         size);
  Bind();
  glBindBuffer(GL_ARRAY_BUFFER, vbo_);
  glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(size), data, usage);
}

void VertexArray::SetElementData(const void *data, const std::size_t size,
                                 const GLenum usage) const {
  ASSERT(data && size,
         "Index data ({}) and size ({}) must be non-null & non-zero!", data,
         size);
  Bind();
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(size), data,
               usage);
}

void VertexArray::SetLayout(const VertexLayout &layout) const {
  ASSERT(layout.stride, "Layout stride must be set");
  Bind();
  glBindBuffer(GL_ARRAY_BUFFER, vbo_);

  for (const auto &element : layout.elements) {
    glEnableVertexAttribArray(element.location);
    glVertexAttribPointer(element.location, element.components, element.type,
                          element.normalized,
                          static_cast<GLsizei>(layout.stride),
                          reinterpret_cast<const void *>(element.offset));
  }
}

} // namespace gib