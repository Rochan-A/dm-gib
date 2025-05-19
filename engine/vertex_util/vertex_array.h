#pragma once

#include <vector>

#define GLAD_GL_IMPLEMENTATION
#include "third_party/glad/glad.h"

#include "engine/vertex_util/types.h"
#include "util/report/macros.h"

namespace gib {
// Ref: https://learnopengl.com/Getting-started/Hello-Triangle

class VertexArray {
public:
  struct VertexAttribute {
    VertexAttribute(unsigned int index, unsigned int size, unsigned int type,
                    bool normalized, size_t stride, void *pointer)
        : index(index), size(size), type(type), normalized(normalized),
          stride(stride), pointer(pointer) {}
    unsigned int index;
    unsigned int size;
    unsigned int type;
    bool normalized;
    size_t stride;
    void *pointer;
  };

  VertexArray();

  ~VertexArray() = default;

  void SetActive();
  void SetDeactive();

  void AddVertexBuffer(const void *data, const size_t &size,
                       const GLenum usage = GL_STATIC_DRAW);

  void AddElementBuffer(const void *data, const size_t &size,
                        const GLenum usage = GL_STATIC_DRAW);

  void AddInstanceVertexBuffer(const void *data, const size_t &size,
                               const GLenum usage = GL_STATIC_DRAW);

  void AddVertexAttribute(const size_t size, const GLenum type,
                          const bool normalized);

  void Apply();

  unsigned int GetVao() const { return vao_; }
  unsigned int GetVbo() const { return vbo_; }
  unsigned int GetEbo() const { return ebo_; }

private:
  // Vertex array obj.
  unsigned int vao_{0};
  // Vertex buffer obj.
  unsigned int vbo_{0};
  // Instance element buffer obj.
  unsigned int instance_vbo_{0};

  // Element buffer obj.
  unsigned int ebo_{0};

  std::vector<VertexAttribute> vertex_attrs_;

  // Next position in VAO.
  unsigned int next_vao_attr_pos_{0};

  unsigned int stride_{0};
};

} // namespace gib