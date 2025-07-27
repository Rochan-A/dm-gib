#pragma once

#define GLAD_GL_IMPLEMENTATION
#include "third_party/glad/glad.h"

#include "engine/shaders/compiler.h"
#include "engine/shaders/types.h"
#include "util/report/report.h"

namespace gib {

class Shader {
public:
  Shader() = default;
  explicit Shader(const ShaderSource &source);
  Shader(const ShaderSource &source1, const ShaderSource &source2);

  ~Shader() = default;

  [[nodiscard]] inline unsigned int GetProgramId() const {
    return shader_program_;
  }

  void Link();

  Shader(Shader &&other) = delete;
  Shader &operator=(Shader &&other) = delete;

  Shader(const Shader &) = delete;
  const Shader &operator=(const Shader &) = delete;

protected:
  ShaderCompiler shader_compiler_{};
  unsigned int shader_program_{0};
};

} // namespace gib