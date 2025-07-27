#pragma once

#define GLAD_GL_IMPLEMENTATION
#include "third_party/glad/glad.h"

namespace gib {

enum class ShaderType : GLenum {
  VERTEX = GL_VERTEX_SHADER,
  FRAGMENT = GL_FRAGMENT_SHADER,
  GEOMETRY = GL_GEOMETRY_SHADER,
};

struct ShaderSource {
  ShaderSource(const char *shader, const ShaderType type,
               const bool is_path = false)
      : shader(shader), type(type), is_path(is_path) {}

  const char *shader;
  ShaderType type;
  bool is_path;
};

} // namespace gib