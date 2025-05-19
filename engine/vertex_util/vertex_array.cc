#include "engine/vertex_util/vertex_array.h"

namespace gib {

VertexArray::VertexArray() { glGenVertexArrays(1, &vao_); }

void VertexArray::SetActive() { glBindVertexArray(vao_); }

void VertexArray::SetDeactive() { glBindVertexArray(vao_); }

void VertexArray::AddVertexBuffer(const void *data, const size_t &size,
                                  const GLenum usage) {
  ASSERT(data != nullptr && size > 0, "Expected data != nullptr & size > 0!");
  if (vbo_ > 0) {
    THROW_FATAL("VBO already set!");
  }

  SetActive();
  glGenBuffers(1, &vbo_);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_);
  glBufferData(GL_ARRAY_BUFFER, size, data, usage);
}

void VertexArray::AddElementBuffer(const void *data, const size_t &size,
                                   const GLenum usage) {
  ASSERT(data != nullptr && size > 0, "Expected data != nullptr & size > 0!");
  if (ebo_ > 0) {
    THROW_FATAL("EBO already set!");
  }
  SetActive();
  glGenBuffers(1, &ebo_);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, usage);
}

void VertexArray::AddInstanceVertexBuffer(const void *data, const size_t &size,
                                          const GLenum usage) {
  THROW_FATAL("TODO!");
  ASSERT(data != nullptr && size > 0, "Expected data != nullptr & size > 0!");
  if (instance_vbo_ > 0) {
    THROW_FATAL("Instance VBO already set!");
  }
  SetActive();
  glGenBuffers(1, &instance_vbo_);
  glBindBuffer(GL_ARRAY_BUFFER, instance_vbo_);
  glBufferData(GL_ARRAY_BUFFER, size, data, usage);
}

void VertexArray::AddVertexAttribute(const size_t size, const GLenum type,
                                     const bool normalized) {
  ASSERT(size > 0, "Expected size > 0!");
  vertex_attrs_.emplace_back(next_vao_attr_pos_,
                             static_cast<unsigned int>(size), type, normalized,
                             stride_, nullptr);
  ++next_vao_attr_pos_;
  stride_ += size * sizeof(type);
}

void VertexArray::Apply() {
  ASSERT(vertex_attrs_.size() > 0,
         "Expected atleast one vertex attribute to be added!");
  SetActive();
  int offset = 0;
  for (const auto &attr : vertex_attrs_) {
    glVertexAttribPointer(attr.index, attr.size, attr.type, attr.normalized,
                          stride_, static_cast<const char *>(nullptr) + offset);
    glEnableVertexAttribArray(attr.index);
    offset += attr.size * sizeof(attr.type);
  }
  vertex_attrs_.clear();
  stride_ = 0;
}

} // namespace gib