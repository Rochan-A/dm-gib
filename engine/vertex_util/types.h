#pragma once

#define GLAD_GL_IMPLEMENTATION
#include "third_party/glad/glad.h"

#include <cstddef>

namespace gib {

enum class BufferType : GLenum {
  VERTEX = GL_ARRAY_BUFFER,
  ELEMENT = GL_ELEMENT_ARRAY_BUFFER,
};

} // namespace gib