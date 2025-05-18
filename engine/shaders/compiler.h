#pragma once

#include "engine/shaders/types.h"
#include "util/report/macros.h"
#include <cstddef>
#include <vector>

#define GLAD_GL_IMPLEMENTATION
#include "third_party/glad/glad.h"

namespace gib {

inline const char *LoadShader(const ShaderSource &source) {
  // TODO
  if (source.is_path) {
    THROW_FATAL("No support for ShaderSource from path!\n{}",
                std::string(source.shader));
  }
  return source.shader;
}

class ShaderCompiler {
public:
  ShaderCompiler() = default;

  ShaderCompiler(const ShaderCompiler &) = delete;
  const ShaderCompiler &operator=(const ShaderCompiler &) = delete;

  ShaderCompiler(ShaderCompiler &&other) = delete;
  ShaderCompiler &operator=(ShaderCompiler &&other) = delete;

  // Loads and compiles shader. Returns shader ID.
  unsigned int LoadAndCompile(const ShaderSource &source);

  // Links all compiled shaders into a shader program, and deletes the shaders.
  // Returns shader program ID.
  unsigned int Link();

private:
  // Compiles shaders.
  unsigned int Compile(const ShaderSource &source, const ShaderType type);

  std::vector<unsigned int> shaders_;
};

} // namespace gib