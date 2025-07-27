#pragma once

#include "engine/shaders/types.h"
#include "util/report/report.h"
#include <cstddef>
#include <fstream>
#include <sstream>
#include <vector>

#define GLAD_GL_IMPLEMENTATION
#include "third_party/glad/glad.h"

namespace gib {

class ShaderCompiler {
public:
  ShaderCompiler() = default;

  ShaderCompiler(const ShaderCompiler &) = delete;
  const ShaderCompiler &operator=(const ShaderCompiler &) = delete;

  ShaderCompiler(ShaderCompiler &&other) = delete;
  ShaderCompiler &operator=(ShaderCompiler &&other) = delete;

  // Loads and compiles shader. Returns shader ID.
  unsigned int LoadAndCompile(const ShaderSource &source);

  // Links all compiled shaders into a shader program, and deletes the
  // shaders. Returns shader program ID.
  unsigned int Link();

private:
  // Compiles shaders.
  static unsigned int Compile(const ShaderSource &source, ShaderType type);

  std::vector<unsigned int> shaders_;
};

} // namespace gib