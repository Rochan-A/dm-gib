#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

#include "engine/materials/material.h"
#include "engine/textures/texture.h"
#include "engine/vertex_util/types.h"
#include "engine/vertex_util/vertex_array.h"
#include "engine/vertex_util/vertex_layout.h"
#include "util/macros.h"

namespace gib {

class Mesh {
public:
  Mesh(VertexArray *vao, std::uint32_t index_count, Material *material)
      : vao_(vao), index_count_(index_count), material_(material) {}

  // Binds material + VAO, then emits glDraw
  void Draw(TextureRegistry &texture_registry) const {
    material_->Bind(texture_registry); // UBO + textures + shader
    vao_->Bind();                      // vertex + index buffers
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(index_count_),
                   GL_UNSIGNED_INT, nullptr);
  }

  // TODO(rochan): Use handles
  [[nodiscard]] const VertexArray *GetVao() const { return vao_; }
  Material *GetMaterial() { return material_; }
  [[nodiscard]] std::uint32_t IndexCount() const { return index_count_; }

  DISALLOW_COPY_AND_ASSIGN(Mesh);

private:
  VertexArray *vao_;
  std::uint32_t index_count_ = 0;
  Material *material_ = {};
};

} // namespace gib