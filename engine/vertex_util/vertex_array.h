#pragma once

#include "util/macros.h"
#include <vector>

#define GLAD_GL_IMPLEMENTATION
#include "third_party/glad/glad.h"

#include "engine/vertex_util/types.h"
#include "engine/vertex_util/vertex_layout.h"
#include "util/report/report.h"

namespace gib {

class VertexArray {
public:
  VertexArray();
  ~VertexArray();

  // Set vertex data.
  void SetVertexData(const void *data, std::size_t size,
                     const GLenum usage = GL_STATIC_DRAW) const;

  // Set element data (optional)
  void SetElementData(const void *data, std::size_t size,
                      const GLenum usage = GL_STATIC_DRAW) const;

  // Bind a ready-made layout to this VAO
  void SetLayout(const VertexLayout &layout) const;

  void Bind() const { glBindVertexArray(vao_); }
  static void Unbind() { glBindVertexArray(0); }

  [[nodiscard]] GLuint GetVao() const { return vao_; }
  [[nodiscard]] GLuint GetVbo() const { return vbo_; }
  [[nodiscard]] GLuint GetEbo() const { return ebo_; }

  DISALLOW_COPY_AND_ASSIGN(VertexArray);

private:
  GLuint vao_{0};
  GLuint vbo_{0};
  GLuint ebo_{0};
};

} // namespace gib